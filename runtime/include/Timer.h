#pragma once

#include <vector>
#include <algorithm>

enum class TimerEventType {
    Every,
    Equals,
    GreaterThan,
    LessThan
};

class TimerEvent {
public:
    int id; // ID of the event
    int time; // Total Time in milliseconds of this event
    int checkTime; // Time to check for the event
    TimerEventType type; // Type of event
    bool triggered; // Whether the condition has been met
    bool alreadyChecked; // Whether the event has already been checked this frame

    TimerEvent(int id, int checkTime, TimerEventType type) 
        : id(id), time(0), checkTime(checkTime), type(type), 
          triggered(false), alreadyChecked(false) {}

    void Update(int deltaTime) {
        time += deltaTime;
    }

    void Reset() {
        time = 0;
        triggered = false;
        alreadyChecked = false;
    }
};

class Timer {
private:
    int time;
    int deltaTime;
    std::vector<TimerEvent> events;

public:
    Timer() : time(0), deltaTime(0) {}

    void Update(double dt) {
        deltaTime = static_cast<int>(dt * 1000.0 + (dt >= 0.0 ? 0.5 : -0.5));
        if (deltaTime < 0) {
            deltaTime = 0;
        }
        time += deltaTime;

        // Update event times
        for (auto& evt : events) {
            evt.alreadyChecked = false;
            evt.Update(deltaTime);
        }
    }

    void SetTime(int time) {
        this->time = time;
    }

    int GetTime() const {
        return time;
    }

    int GetHundreds() const {
        return (time / 10) % 100;
    }

    int GetSeconds() const {
        return (time / 1000) % 60;
    }

    int GetMinutes() const {
        return (time / 60000) % 60;
    }

    int GetHours() const {
        return (time / 3600000) % 24;
    }

    bool CheckEvent(int evtID, int checkTime, TimerEventType type) {
        // Check if event exists, if it does not, add it
        auto it = std::find_if(events.begin(), events.end(), 
            [evtID](const TimerEvent& evt) { return evt.id == evtID; });

        if (it == events.end()) {
            events.emplace_back(evtID, checkTime, type);
            events.back().Update(deltaTime);
            it = events.end() - 1;
        }

        TimerEvent& evt = *it;

        if (evt.type == TimerEventType::Equals) {
            if (time < evt.checkTime) {
                evt.triggered = false;
                evt.alreadyChecked = false;
            }
        }

        // Check if the event has already been checked this frame
        if (evt.alreadyChecked) {
            return false;
        }

        // Check conditions
        switch (type) {
            case TimerEventType::Every:
                if (evt.checkTime <= 0 || evt.time >= evt.checkTime) {
                    if (evt.checkTime > 0) {
                        evt.time %= evt.checkTime;
                    }
                    evt.triggered = true;
                    evt.alreadyChecked = true;
                    return true;
                }
                break;
            case TimerEventType::Equals:
                if (time >= evt.checkTime && !evt.triggered) {
                    evt.triggered = true;
                    evt.alreadyChecked = true;
                    return true;
                }
                break;
            case TimerEventType::GreaterThan:
                if (time > evt.checkTime) {
                    evt.triggered = true;
                    return true;
                }
                break;
            case TimerEventType::LessThan:
                if (time < evt.checkTime) {
                    evt.triggered = true;
                    return true;
                }
                break;
        }

        return false;
    }
}; 