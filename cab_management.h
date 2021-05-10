#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <list>
#include <map>
#include <iostream>

using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
using BookingID_t = int32_t;

// Cab possible state
enum eCabState
{
    IDLE,
    ON_TRIP
};

// Predefined cities
enum eCity
{
    Pune,
    Mumbai,
    Delhi,
    Nagpur,
    Bangaluru,
    __Last
};

struct Trip
{
    BookingID_t bookingId;
    eCity source;
    eCity destination;
    TimePoint startTrip;
    TimePoint endTrip;

    friend std::ostream& operator<<(std::ostream& os, const Trip& trip)
    {
        os << "bookingId:" << trip.bookingId
            << " source:" << trip.source
            << " destination:" << trip.destination;
        return os;
    }
};

struct BookingMetaData
{
    BookingID_t bookingId;
    std::string cabId;
};

class Cab
{
public:
    Cab(const std::string& cabId, eCity currentCity): m_cabId(cabId), m_currentCity(currentCity)
    {
    }

    bool InTransit() const
    {
        return m_state == eCabState::ON_TRIP;
    }

    eCity CurrentCity() const
    {
        if(InTransit())
            return eCity::__Last;

        return m_currentCity;
    }

    void ChangeCity(eCity city)
    {
        m_currentCity = city;
    }

    void ChangeState(eCabState state)
    {
        m_state = state;
    }

    std::string GetId() const{
        return m_cabId;
    }

    void Book(BookingID_t bookingId, eCity destination)
    {
        Trip trip = { bookingId, m_currentCity, destination };
        m_trips.emplace(bookingId, std::move(trip));
    }

    void Start(BookingID_t bookingId)
    {
        m_trips[bookingId].startTrip = std::chrono::system_clock::now();
        m_state = eCabState::ON_TRIP;
    }

    void End(BookingID_t bookingId)
    {
        auto tripItr = m_trips.find(bookingId);
        if(tripItr == m_trips.end())
            return;

        tripItr->second.endTrip = std::chrono::system_clock::now();
        m_currentCity = tripItr->second.destination;
        m_state = eCabState::IDLE;
        m_lastSeenTime = tripItr->second.endTrip;
    }

    void Cancel(BookingID_t bookingId)
    {
        auto tripItr = m_trips.find(bookingId);
        if(tripItr == m_trips.end())
            return;

        m_trips.erase(bookingId);
        m_state = eCabState::IDLE;
    }

    auto GetLastSeenTime()
    {
        return m_lastSeenTime;
    }

    int64_t IdleTime(time_t startTime, time_t endTime) const
    {
        int64_t idleTime = 0;
        auto t1 = std::chrono::system_clock::from_time_t(startTime);
        auto t2 = std::chrono::system_clock::from_time_t(endTime);
        auto totalNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1);
        std::for_each(m_trips.begin(), m_trips.end(),
            [&](const auto &pair){
                if(t1 < pair.second.startTrip && pair.second.endTrip < t2)
                {
                    auto nanoSec = std::chrono::duration_cast<std::chrono::nanoseconds>(pair.second.endTrip - pair.second.startTrip);
                    idleTime += nanoSec.count();
                }
            }
        );

        return totalNanos.count() - idleTime;
    }

    std::vector<Trip> GetTrips() const
    {
        std::vector<Trip> trips;
        for(const auto& trip :  m_trips)
            trips.emplace_back(trip.second);

        return trips;
    }

    void Reset()
    {
        m_state = eCabState::IDLE;
        m_currentCity = eCity::__Last;
        m_trips.clear();
        m_lastSeenTime = TimePoint();
    }

private:
    std::string m_cabId;
    eCabState m_state = eCabState::IDLE;
    eCity m_currentCity = eCity::__Last;
    std::unordered_map<BookingID_t, Trip> m_trips;
    TimePoint m_lastSeenTime;
};


class CabManager
{
public:
    bool RegisterCab(const std::string& cabId, eCity city)
    {
        if(m_cabs.find(cabId) != m_cabs.end())
            return false;

        Cab * newCab = new Cab(cabId, city);
        m_cabs.emplace(cabId, newCab);
        return true;
    }

    BookingMetaData QueryBooking(eCity source, eCity destination)
    {
        auto cab = Book(source, destination);
        if(cab == nullptr)
            return BookingMetaData();

        // Generate Booking ID
        auto bookingId = rand();
        cab->Book(bookingId, destination);
        m_bookings[bookingId] = cab;
        return {bookingId, cab->GetId()};
    }

    void Start(BookingID_t bookingId)
    {
        m_bookings[bookingId]->Start(bookingId);
    }

    void End(BookingID_t bookingId)
    {
        m_bookings[bookingId]->End(bookingId);
    }

    const Cab* GetCab(const std::string& cabId)
    {
        return m_cabs[cabId];
    }

private:
    Cab* Book(eCity source, eCity destination)
    {
        // Get all the cabs having current city = source sorted by their last seen
        std::map<TimePoint, std::vector<Cab*>> availableCabs;
        for(const auto& cabItr: m_cabs)
        {
            if(cabItr.second->CurrentCity() == source)
            {
                availableCabs[cabItr.second->GetLastSeenTime()].push_back(cabItr.second);
            }
        }

        if(availableCabs.empty())
            return nullptr; // no cabs available at this time

        auto maxWaitedCabs = availableCabs.begin();
        auto cabs = maxWaitedCabs->second;
        if(cabs.size() > 1)
        {
            // more than one cab available for booking
            return cabs[rand() % cabs.size()];
        }
        else{
            return cabs[0];
        }
    }
    std::unordered_map<std::string, Cab*> m_cabs;
    std::unordered_map<BookingID_t, Cab*> m_bookings;
};