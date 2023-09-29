#ifndef ROBOT_ACTION__H
#define ROBOT_ACTION__H

#include <string>
#include <atomic>
#include <memory>
#include <vector>
#include <list>
#include <functional>
#include <sstream>

namespace robotaction {

using std::string, std::atomic, std::shared_ptr, std::list, std::vector;

enum class ActionResultEnum: uint8_t {
    FAILURE = 0,
    SUCCESS = 1,
    CONTINUE = 2,
};

class ActionError {
public:
    ActionError() {}
    ActionError(
        const string& error_description,
        const string& action_name,
        const string& action_description):
            error_description(error_description),
            action_name(action_name),
            action_description(action_description) {}
    string get_error_description() const { return error_description; }
    string get_action_name() const { return action_name; }
    string get_action_description() const { return action_description; }
    string to_string() const { return error_description + " " + action_name + " " + action_description; }
protected:
    string error_description;
    string action_name;
    string action_description;
};


class ActResult {
public:
    ActResult(
        ActionResultEnum value):
            _value(value) {}
    ActResult(
        const ActionError& error):
            _value(ActionResultEnum::FAILURE),
            _error(error) {}
    ActResult(
        const string& error_description,
        const string& action_name,
        const string& action_description):
            _value(ActionResultEnum::FAILURE),
            _error(error_description, action_name, action_description) {}

    ActResult(
        const ActionResultEnum value,
        const string& error_description,
        const string& action_name,
        const string& action_description):
            _value(value),
            _error(error_description, action_name, action_description) {}

    static const ActResult SUCCESS;
    static const ActResult CONTINUE;

    static ActResult FAILURE(const ActionError& error) {
        return ActResult(error);
    }

    static ActResult FAILURE(
        const string& error_description,
        const string& action_name,
        const string& action_description)
    {
        return ActResult(error_description, action_name, action_description);
    }

    ActionResultEnum get_value() const {
        return _value;
    }

    ActionError get_error() const {
        return _error;
    }

    bool is_continue() const { return get_value() == ActionResultEnum::CONTINUE; }
    bool is_failure() const { return get_value() == ActionResultEnum::FAILURE; }
    bool is_success() const { return get_value() == ActionResultEnum::SUCCESS; }

    ActionResultEnum operator() () const {
        return _value;
    }

    bool operator == (const ActResult& other) const {
        return _value == other._value;
    }

    bool operator != (const ActResult& other) const {
        return _value != other._value;
    }

    bool operator == (ActionResultEnum other) const {
        return _value == other;
    }

    bool operator != (ActionResultEnum other) const {
        return _value != other;
    }

    string to_string() const;
    string to_short_string() const;

protected:
    ActionResultEnum _value;
    ActionError _error;
};

template <typename State, typename Command>
class Action {
public:

    using Ptr = shared_ptr<Action<State, Command>>;

    Action(const string& name="Action"):
        name(name),
        first_time(true) {}

    virtual ~Action() {}

    string get_name() const { return name; }

    virtual void reset() {
        first_time = true;
        override_return = -1;
    }

    void set_override_return(ActionResultEnum r) {
        override_return = static_cast<int>(r);
    }

    bool was_return_overridden() const {
        return override_return != -1;
    }

    ActResult rtf(
        double robot_time,
        const State& state,
        Command& command);

    virtual string to_string() const;

protected:

    virtual ActResult act(
        double robot_time,
        const State& state,
        Command& command)
    {
        return ActResult::SUCCESS;
    }

private:

    string name;
    bool first_time;
    atomic<int> override_return = -1;
};

template <typename State, typename Command, typename Robot>
class RobotActionRunner {
public:

    // void start();
    // void stop();

    ActionResult run_action_wait(shared_ptr<Action<State, Command>> action) {
        
    }

    void robot_loop_action() {

    }
    
    void run_robot_loop() {

    }
};

template <typename State, typename Command>
class Null: public Action<State, Command> {
public:

    Null(const string& name = "Null"): 
        Action<State, Command>(name) {}
    
    string to_string() const override {
        return "<Null " + Action<State, Command>::to_string() + ">";
    }

protected:

    ActResult act(
        double robot_time,
        const State& state,
        Command& command) override 
    {
        return ActResult::SUCCESS;
    }
};


template <typename State, typename Command>
class Contain: public Action<State, Command> {
public:

    Contain(
        Action<State, Command>::Ptr contained_action,
        const string& name = "Contain"):
            Action<State, Command>(name),
            contained_action(contained_action) {}

    void reset() override {
        Action<State, Command>::reset();
        contained_action->reset();
    }

    Action<State, Command>::Ptr get_contained_action() const { 
        return contained_action; 
    }

    string to_string() const override {
        return "<Contain " + Action<State, Command>::to_string() + " [" + contained_action->to_string() + "]>";
    }

protected:

    ActResult act(
        double robot_time,
        const State& state,
        Command& command) override
    {
        return contained_action->rtf(robot_time, state, command);
    }

    Action<State, Command>::Ptr contained_action;
};


template <typename State, typename Command>
class Timeout: public Contain<State, Command> {
public:

    Timeout(
        double duration_seconds,
        Action<State, Command>::Ptr action,
        bool timeout_fails = true,
        const string& name = "Timeout"):
            Contain<State, Command>(action, name),
            duration_seconds(duration_seconds),
            _t0(0),
            timeout_fails(timeout_fails) {}

    double get_duration_seconds() const { 
        return duration_seconds; 
    }
    
    bool get_timeout_fails() const { 
        return timeout_fails; 
    }

    void reset() override {
        Contain<State, Command>::reset();
        _t0 = 0;
    }

    string to_string() const override {
        return "<Timeout (" + std::to_string(duration_seconds) + ") seconds " + Contain<State, Command>::to_string() + ">";
    }

protected:

    double duration_seconds;
    double _t0;
    bool timeout_fails;

    ActResult act(
        double robot_time,
        const State& state,
        Command& command) override
    {
        if (_t0 == 0) {
            _t0 = robot_time;
        }

        if (duration_seconds > 0) {
            if (robot_time > (_t0 + duration_seconds)) {
                return timeout_fails ?
                    ActResult::FAILURE("timeout", *this) :
                    ActResult::SUCCESS;
            }
        }

        if (this->contained_action == nullptr) {
            return ActResult::CONTINUE;
        }

        return Contain<State, Command>::act(robot_time, state, command);
    }
};


template <typename State, typename Command>
class NeverFail: public Contain<State, Command> {
public:

    NeverFail(
        Action<State, Command>::Ptr action,
        const string& name = "NeverFail"):
            Contain<State, Command>(action, name) {}

    string to_string() const override {
        return "<NeverFail " + Contain<State, Command>::to_string() + ">";
    }

protected:

    ActResult act(
        double robot_time,
        const State& state,
        Command& command) override
    {
        ActResult ar = Contain<State, Command>::act(robot_time, state, command);
        if (ar.is_failure()) {
            return ActResult::SUCCESS;
        }
        return ar;
    }
};


template <typename State, typename Command>
class InvertResult: public Contain<State, Command> {
public:

    InvertResult(
        Action<State, Command>::Ptr action,
        const string& name = "InvertResult"):
            Contain<State, Command>(action, name) {}

    string to_string() const override {
        return "<InvertResult " + Contain<State, Command>::to_string() + ">";
    }

protected:

    ActResult act(
        double robot_time,
        const State& state,
        Command& command) override
    {
        ActResult ar = Contain<State, Command>::act(robot_time, state, command);
        if (ar.is_failure()) {
            return ActResult::SUCCESS;
        } else if (ar.is_success()) {
            return ActResult::FAILURE("InvertResult", *this);
        }
        return ar;
    }
};


template <typename State, typename Command>
class Repeat: public Contain<State, Command> {
public:

    Repeat(
        Action<State, Command>::Ptr action,
        int num_times,
        const string& name = "Repeat"):
            Contain<State, Command>(action, name),
            num_times(num_times),
            times_performed(0) {}

    void reset() override {
        Contain<State, Command>::reset();
        times_performed = 0;
    }

    string to_string() const override {
        return "<Repeat x " + std::to_string(num_times) + " " + Contain<State, Command>::to_string() + ">";
    }

protected:

    int num_times;
    int times_performed;

    ActResult act(
        double robot_time,
        const State& state,
        Command& command) override
    {
        ActResult ar = Contain<State, Command>::act(robot_time, state, command);
        if (ar.is_success()) {
            times_performed += 1;
            if (times_performed == num_times) {
                return ActResult::SUCCESS;
            } else {
                this->get_contained_action()->reset();
                return ActResult::CONTINUE;
            }
        }

        return ar;
    }
};


template <typename State, typename Command>
class Composite: public Action<State, Command> {
public:

    Composite(
        const vector<shared_ptr<Action<State, Command>>>& action_list,
        const string& name = "Composite"):
            Action<State, Command>(name),
            action_list(action_list) {}

    void reset() override {
        Action<State, Command>::reset();
        for (auto a: action_list) {
            a->reset();
        }
        current_action_pos = 0;
    }

    string to_string() const override {
        std::stringstream sts;
        sts << "<Composite " + Action<State, Command>::to_string() + "(" << action_list.size() << ") [";\
        bool first = true;
        for (auto a: action_list) {
            if (!first) {
                sts << ", ";
            }
            first = false;
            sts << a->to_string();
        }
        sts << "]>";
        return sts.str();
    }

protected:

    vector<shared_ptr<Action<State, Command>>> action_list;
    size_t current_action_pos = 0;
};


template <typename State, typename Command>
class Sequence: public Composite<State, Command> {
public:

    Sequence(
        const vector<shared_ptr<Action<State, Command>>>& action_list,
        const string& name = "Sequence"):
            Composite<State, Command>(action_list, name) {}

    string to_string() const override {
        return "<Sequence " + Composite<State, Command>::to_string() + ">";
    }

protected:

    ActResult act(
        double robot_time,
        const State& state,
        Command& command) override
    {
        shared_ptr<Action<State, Command>> action = this->action_list[this->current_action_pos];
        ActResult ar = action->rtf(robot_time, state, command);

        while (ar.is_success() && this->current_action_pos < this->action_list.size()) {
            this->current_action_pos += 1;
            if (this->current_action_pos < this->action_list.size()) {
                action = this->action_list[this->current_action_pos];
                ar = action->rtf(robot_time, state, command);
            }
        }

        return ar;
    }
};


template <typename State, typename Command>
class Parallel: public Composite<State, Command> {
public:

    Parallel(
        const vector<shared_ptr<Action<State, Command>>>& action_list,
        const string& name):
            Composite<State, Command>(action_list, name),
            parallel_actions(action_list) {}

    void reset() override {
        Composite<State, Command>::reset();
        parallel_actions = this->action_list;
    }

    string to_string() const override {
        return "<Parallel " + Composite<State, Command>::to_string() + ">";
    }

protected:

    vector<shared_ptr<Action<State, Command>>> parallel_actions;

    void remove_actions(const list<shared_ptr<Action<State, Command>>>& actions) {
        for (auto action: actions) {
            auto pos = std::find(parallel_actions.begin(), parallel_actions.end(), action);
            parallel_actions.erase(pos);
        }
    }
};


template <typename State, typename Command>
class And: public Parallel<State, Command> {
public:

    And(
        const vector<shared_ptr<Action<State, Command>>>& action_list,
        const string& name = "And"):
            Parallel<State, Command>(action_list, name) {}

    string to_string() const override {
        return "<And " + Parallel<State, Command>::to_string() + ">";
    }

protected:

    ActResult act(
        double robot_time,
        const State& state,
        Command& command) override
    {
        list<shared_ptr<Action<State, Command>>> actions_to_remove;

        // propagate rtf to every action
        for (auto action: this->parallel_actions) {
            ActResult ar = action->rtf(robot_time, state, command);
            if (ar.is_success()) {
                // Child succeeded: remove from list.
                actions_to_remove.push_back(action);
            } else if (ar.is_failure()) {
                // Child failed: we fail. Propagate error.
                return ar;
            }
        }

        remove_actions(actions_to_remove);

        // If no more actions: we succeeded.
        if (this->parallel_actions.empty()) {
            return ActResult::SUCCESS;
        }

        return ActResult::CONTINUE;
    }
};


template <typename State, typename Command>
class Failover: public Composite<State, Command> {
public:

    Failover(
        const vector<shared_ptr<Action<State, Command>>>& action_list,
        const string& name = "Failover"):
            Composite<State, Command>(action_list, name) {}

    string to_string() const override {
        return "<Failover " + Composite<State, Command>::to_string() + ">";
    }

protected:

    ActResult act(
        double robot_time,
        const State& state,
        Command& command) override
    {
        shared_ptr<Action<State, Command>> action = this->action_list[this->current_action_pos];
        ActResult ar = action->rtf(robot_time, state, command);

        while (ar.is_failure() && this->current_action_pos < this->action_list.size()) {
            this->current_action_pos += 1;
            if (this->current_action_pos < this->action_list.size()) {
                action = this->action_list[this->current_action_pos];
                ar = action->rtf(robot_time, state, command);
            }
        }

        return ar;
    }
};


template <typename State, typename Command>
class Or: public Parallel<State, Command> {
public:

    Or(
        const vector<shared_ptr<Action<State, Command>>>& action_list,
        const string& name = "Or"):
            Parallel<State, Command>(action_list, name) {}

    string to_string() const override {
        return "<Or " + Parallel<State, Command>::to_string() + ">";
    }

protected:

    ActResult act(
        double robot_time,
        const State& state,
        Command& command) override
    {
        list<shared_ptr<Action<State, Command>>> actions_to_remove;

        // propagate rtf to every action
        for (auto action: this->parallel_actions) {
            ActResult ar = action->rtf(robot_time, state, command);
            if (ar.is_success()) {
                // Child succeeded: we succeed.
                return ar;
            } else if (ar.is_failure()) {
                // Child failed: remove from list.
                actions_to_remove.push_back(action);
            }
        }

        remove_actions(actions_to_remove);

        // If no more actions: we failed.
        if (this->parallel_actions.empty()) {
            return ActResult::FAILURE("no child action succeeded", *this);
        }

        return ActResult::CONTINUE;
    }
};


template <typename State, typename Command>
class StateMachine: public Action<State, Command> {
public:

    /**
     * A function that takes the current action and 
     * the result of its final act() method, and returns
     * a new Action to run.
     * 
     * When any action is done, the StateMachine will run 
     * this function to get the next action to run. When that
     * is nullptr, then this StateMachine will return SUCCESS.
    */
    using StateTransitionFunction = std::function<
        shared_ptr<Action<State, Command>>(
            shared_ptr<Action<State, Command>>, 
            const ActResult&)
        >;

    StateMachine(
        StateTransitionFunction delegate,
        const string& name = "StateMachine"):
            Action<State, Command>(name),
            delegate(delegate),
            current_action(nullptr) {}

    void reset() override {
        Action<State, Command>::reset();
        current_action = nullptr;
    }

    string to_string() const override {
        return "<StateMachine " + Action<State, Command>::to_string() + ">";
    }

protected:

    StateTransitionFunction delegate;
    shared_ptr<Action<State, Command>> current_action;

    ActResult act(
        double robot_time,
        const State& state,
        Command& command) override
    {
        if (current_action == nullptr) {
            current_action = delegate(nullptr, ActResult::SUCCESS);
        }

        if (current_action == nullptr) {
            return ActResult::SUCCESS;
        }

        auto ar = current_action->rtf(robot_time, state, command);

        if (ar != ActResult::CONTINUE) {
            current_action = delegate(current_action, ar);

            if (current_action == nullptr) {
                return ar;
            }
        }

        return ar;
    }
};


template <typename State, typename Command>
class ContinueForever: public Action<State, Command> {
public:

    ContinueForever(
        const string& name = "ContinueForever"):
            Action<State, Command>(name) {}

    string to_string() const override { 
        return "<ContinueForever " + Action<State, Command>::to_string() + ">"; 
    }

protected:

    ActResult act(
        double robot_time,
        const State& state,
        Command& command) override
    {
        return ActResult::CONTINUE;
    }
};


template <typename State, typename Command>
class Pause: public Action<State, Command> {
public:

    Pause(
        double duration_seconds,
        const string& name = "Pause"):
            Action<State, Command>(name),
            duration_seconds(duration_seconds),
            start_t(0) {}

    double get_duration_seconds() const { 
        return duration_seconds; 
    }

    void reset() override {
        Action<State, Command>::reset();
        start_t = 0;
    }

    string to_string() const override {
        return "<Pause(" + std::to_string(duration_seconds) + ") \"" + Action<State, Command>::to_string() + "\">";
    }

protected:

    double duration_seconds;
    double start_t;

    ActResult act(
        double robot_time,
        const State& state,
        Command& command) override
    {
        if (start_t == 0) {
            start_t = robot_time;
        }

        if ((robot_time - start_t) > duration_seconds) {
            return ActResult::SUCCESS;
        }

        return ActResult::CONTINUE;
    }
};

}; // namespace robotaction

#endif // ROBOT_ACTION__H
