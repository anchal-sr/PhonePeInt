# PhonePeInt
Cab Management Portal.

It has two main classes viz. `Cab` and `CabManager`.
Clients of `CabManager` can query for booking the cab, if booking is possible, it returns the bookingId and cab_id.
<br/>
`CabManager cm;`
<br/>
 `cm.QueryBooking(Pune, Mumbai);`
 
 Client can also get the Cab object by passing the cabId.
 <br/>
 `cm.GetCab(cab_id)`
 Cab Object can used to get the history the cab, total idle time of this cab.
 <br/>
 Logic of cab selection. <br/>
 1. Get the list of the all the cabs availabe in source city. <br/>
 2. This list is sorted by their last seen time. It is possible to have multiple cabs available at the same time point. In this case system returns the random selected cab. Otherwise Cab having max idle time is returned.
 
  <br/>
 There are certain admin methods available for explicity changing the cab properties like its city, state, etc.
