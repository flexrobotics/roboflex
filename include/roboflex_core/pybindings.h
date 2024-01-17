#ifndef ROBOFLEX_CORE_PY_BINDINGS__H
#define ROBOFLEX_CORE_PY_BINDINGS__H

#include <string>
#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/complex.h>
#include <pybind11/embed.h>
#include "roboflex_core/core.h"

namespace py = pybind11;

using namespace roboflex;
using namespace roboflex::core;

// Wow, where to start, this is horrible...
// So I guess pybind11 has a problem in keeping python-side
// instances alive, when those instances are inherited from
// c++ instances (i.e. python-side classes that inherit Message)
// and when those instances are owned by shared_ptr.
// This is a big ugly workaround. Why doesn't this
// also apply to Node? It doesn't seem to... although
// we should keep an eye on it.
//
// Hopefully, at some point, pybind11 will fix this 'for real'.
// Until then, we need the following template specialization.
//
// https://github.com/pybind/pybind11/issues/1546

namespace pybind11::detail {

// Gets the roboflex.core.messages module - this code
// is needed to run both from a python environment,
// and from bazel (the catch code).
inline object get_messages_module()
{
    object o;
    try {
        o = py::module_::import("roboflex.dynoflex");
    } catch (std::exception& e) {
        o = py::module_::import("roboflex.python.dynoflex");
    }
    return o;
}

// Get the DynoFlex class from the messages module
inline object get_dynoflex_class()
{
    return get_messages_module().attr("DynoFlex");
}

// Gets the DynoFlex.method_name method as a c++ object
// that can be used to later construct DynoFlex objects.
inline object get_dynoflex_construction_method(const std::string& method_name)
{
    object m = get_dynoflex_class().attr(method_name.c_str());

    // CHEAP HACK!!! This method will only ever be called by two other places,
    // all in this file. Normally: when a program ends, and the python interpreter
    // gets unloaded, it destructs all extant pybind::objects. However, below
    // you can see that we use static variables inside functions to cache these
    // construction functions (they are expensive to extract). And so when the 
    // c++-level 'module' (or tpu or shared object or whatever) gets unloaded,
    // the destructors will be called again, resulting in an annoying (but harmless)
    // SegmentationFault. But if we do this, it's all ok - basically, that object
    // will never get destructed. A trifle unclean, but maybe slightly less unclean 
    // than before...
    m.inc_ref();

    return m;
}


void initialize_module_loading()
{
    get_dynoflex_construction_method("from_msg");
}

template<>
struct type_caster<std::shared_ptr<Message>> {
    PYBIND11_TYPE_CASTER (std::shared_ptr<Message>, _("Message"));

    using BaseCaster = copyable_holder_caster<Message, std::shared_ptr<Message>>;

    bool load (pybind11::handle src, bool b)
    {
        BaseCaster bc;
        bool success = bc.load (src, b);
        if (!success)
        {
            return false;
        }

        auto py_obj = py::reinterpret_borrow<py::object> (src);
        auto base_ptr = static_cast<MessagePtr> (bc);

        // Construct a shared_ptr to the py::object
        auto py_obj_ptr = std::shared_ptr<object>{
            new object{py_obj},
            [](auto py_object_ptr) {
                // It's possible that when the shared_ptr dies we won't have the
                // gil (if the last holder is in a non-Python thread), so we
                // make sure to acquire it in the deleter.
                gil_scoped_acquire gil;
                delete py_object_ptr;
            }
        };

        value = std::shared_ptr<Message>(py_obj_ptr, base_ptr.get());
        return true;
    }

    std::shared_ptr<Message> get_value() const { return value; }

    static handle cast (std::shared_ptr<Message> base,
                        return_value_policy rvp,
                        handle h)
    {
        if (base == nullptr) {
            return py::none().release();
            //return py::none();
            //return pybind11::cast<pybind11::none>(Py_None);
            //return none().release();
        }

        // Perform a switcheroo: instead of just casting
        // the base a smart pointer (which is what BaseCaster does),
        // we actually use pybind's c++ api to load the
        // roboflex.core.messages.DynoFlex.from_msg method in python,
        // and use that to construct a DynoFlex: this is what will
        // be handled by the client, i.e. in the Node::receive method,
        // where overloaded by python.

        //pybind11::gil_scoped_acquire gil;

        // initialize the dynoflex constructor
        static auto dynoflex_constructor_from_msg = py::detail::get_dynoflex_construction_method("from_msg");

        // cast the message to a python object
        handle base_object = BaseCaster::cast(base, rvp, h);

        // instantiate a dynoflex object.
        object dynoflex_object = dynoflex_constructor_from_msg(base_object);

        // Tell Python that we've created the object. If omitted, it segfaults.
        dynoflex_object.inc_ref();

        // decrement the reference count of the base object, since we're
        // encapsulating it in a shared pointer, which should take over
        base_object.dec_ref();

        return dynoflex_object;

        //return BaseCaster::cast(base, rvp, h);
    }
};

template <>
struct is_holder_type<Message, std::shared_ptr<Message>> : std::true_type {};

inline MessagePtr dynoflex_from_object(py::object m)
{
    // Get the DynoFlex.from_data method. This might be slow,
    // so make it static.
    static auto dynoflex_constructor_from_data = py::detail::get_dynoflex_construction_method("from_data");

    auto dynoflex_object = dynoflex_constructor_from_data(m);

    // it is unclear exactly when we should release the gil.
    // I'll assume that earlier is better: here seems to work.
    // UPDATE: NO, NOT HERE! typer.load seems to need the gil
    // pybind11::gil_scoped_release gil2;

    // This CANNOT BE STATIC! The typer object retains the value,
    // and that will cause a leak of the last object. Safer
    // to instantiate a new one here.
    auto typer = py::detail::type_caster<MessagePtr>();

    // Use the typer to convert the 'pure python dynoflex object'
    // to a MessagePtr.
    bool cast_succeeded = typer.load(dynoflex_object, false);

    // it is unclear exactly when we should release the gil.
    // I'll assume that earlier is better: here seems to work.
    pybind11::gil_scoped_release gil2;

    if (!cast_succeeded) {
        throw std::runtime_error("Cast of python object to dynoflex failed!");
    }
    MessagePtr dynoflex_wrapper = typer.get_value();
    if (dynoflex_wrapper == nullptr) {
        throw std::runtime_error("Cast of python object to dynoflex succeeded, but then was null!");
    }

    return dynoflex_wrapper;
}

} // namespace pybind11::detail


namespace roboflex::core {

// Allows inheritance in Python from Node
// This is a bit complex: as described in 
// https://pybind11.readthedocs.io/en/stable/advanced/classes.html#combining-virtual-functions-and-inheritance
template <class NodeBase = Node> 
class PyNode: public NodeBase {
public:
    using NodeBase::NodeBase;

    /* Trampoline (need one for each virtual function) */
    void receive(MessagePtr m) override {
        // https://pybind11.readthedocs.io/en/stable/advanced/misc.html#global-interpreter-lock-gil
        // py::gil_scoped_acquire acquire;
        PYBIND11_OVERRIDE(
            void,      /* Return type */
            NodeBase,  /* Parent class */
            receive,   /* Name of function in C++ (must match Python name) */
            m          /* Argument(s) */
        );
    }

    void receive_from(MessagePtr m, const Node& from) override {
        PYBIND11_OVERRIDE(void, NodeBase, receive_from, m, from);
    }

    virtual NodePtr connect(NodePtr node) override {
        PYBIND11_OVERRIDE(NodePtr, NodeBase, connect, node);
    }

    virtual void disconnect(NodePtr node) override {
        PYBIND11_OVERRIDE(void, NodeBase, disconnect, node);
    }

    virtual string to_string() const override {
        PYBIND11_OVERRIDE(string, NodeBase, to_string, );
    }

    MessagePtr handle_rpc(MessagePtr rpc_message) override {
        PYBIND11_OVERRIDE(MessagePtr, NodeBase, handle_rpc, rpc_message);
    }
};

// Allows inheritance in Python from RunnableNode
template <class RunnableNodeBase = RunnableNode> 
class PyRunnableNode: public PyNode<RunnableNodeBase> {
public:
    using PyNode<RunnableNodeBase>::PyNode;

    void child_thread_fn() override {
        PYBIND11_OVERRIDE(void, RunnableNodeBase, child_thread_fn, );
    }

    void start() override {
        PYBIND11_OVERRIDE(void, RunnableNodeBase, start, );
    }

    void stop() override {
        PYBIND11_OVERRIDE(void, RunnableNodeBase, stop, );
    }
};

} // namespace roboflex::core

#endif // ROBOFLEX_CORE_PY_BINDINGS__H
