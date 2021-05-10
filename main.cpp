
#include "cab_management.h"
#include <thread>

int main()
{
    using namespace std::chrono_literals;
    std::cout << "CabManager Starting ......" << std::endl;
    srand(time(0));
    auto startTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    // Register three Cabs for Pune City
    CabManager cm;
    cm.RegisterCab("C1", Pune);
    cm.RegisterCab("C2", Pune);
    cm.RegisterCab("C3", Pune);

    // Query for booking, we should get any random cab this time.
    auto bookingDetail = cm.QueryBooking(Pune, Mumbai);
    std::cout << bookingDetail.bookingId << " " << bookingDetail.cabId << std::endl;
    cm.Start(bookingDetail.bookingId);
    std::string confimedId = bookingDetail.cabId;

    auto bookingDetail1 = cm.QueryBooking(Pune, Mumbai);
    std::cout << bookingDetail1.bookingId << " " << bookingDetail1.cabId << std::endl;

    // Since confimedId has started the trip, it can't be booked till end of the trip.
    assert(confimedId != bookingDetail1.cabId);

    cm.End(bookingDetail.bookingId);
    cm.Start(bookingDetail1.bookingId);

    auto bookingDetail2 = cm.QueryBooking(Pune, Mumbai);
    std::cout << bookingDetail2.bookingId << " " << bookingDetail2.cabId << std::endl;

    // confimedId has completed the trip, since it has least idle time, so it can't be booked because we still have
    // one cab waiting
    assert(confimedId != bookingDetail2.cabId);
    assert(cm.GetCab(confimedId)->CurrentCity() == Mumbai);
    assert(cm.GetCab(bookingDetail1.cabId)->InTransit() == true);
    assert(cm.GetCab(bookingDetail2.cabId)->InTransit() == false);

    cm.End(bookingDetail1.bookingId);

    assert(cm.GetCab(bookingDetail1.cabId)->CurrentCity() == Mumbai);
    assert(cm.GetCab(bookingDetail1.cabId)->InTransit() == false);

    std::this_thread::sleep_for(5s);

    // Now two cabs are in Mumbai, but query should return the first cab which has completed the Pune-Mumbai
    // trip first.
    bookingDetail = cm.QueryBooking(Mumbai, Bangaluru);
    assert(confimedId == bookingDetail.cabId);
    cm.Start(bookingDetail.bookingId);
    cm.End(bookingDetail.bookingId);

    auto * cabAtBanglore = cm.GetCab(confimedId);
    // Get History of this cab
    auto trips = cabAtBanglore->GetTrips();
    // This cab has completed two trips
    assert(trips.size() == 2);

    // Since we have only cab available in Bangaluru, it must be the same which completed the trip
    bookingDetail = cm.QueryBooking(Bangaluru, Pune);
    assert(confimedId == bookingDetail.cabId);

    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::cout << "idleTime of cabId:" << confimedId << " is " << cabAtBanglore->IdleTime(startTime, now) << "nano secs" << std::endl;
    std::cout << "idleTime of cabId:" << bookingDetail2.cabId << " is " << cm.GetCab(bookingDetail2.cabId)->IdleTime(startTime, now)
            << "nano secs" << std::endl;

}