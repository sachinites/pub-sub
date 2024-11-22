# pub-sub
A PUB-SUB library in C/C++

Functionalities :
====================
Coordinator can accept msgs from Subs via multiple IPCs : msgQ, Net Sockets ( UDP / TCP ).
Subscribers which are in same Virtual Address Space ( Diifferent threads of same process ) must be able to commuincate via Callbacks.
Subscribers which are on same machine must be able to communicate via shared memory Or Unix-Sockts Or msgQs
Publishers and Subscribers can Dynamically join and leave
Coordinator must be able recover from process restarts
Coordinator must support message prioritization
When new subscriber Join, he can request bulk transfer of messages from Coordinator
Provide real-time dashboards showing active publishers, subscribers, message rates
Log all events for auditing and debugging.
Scale the distributor by implementing clustering, allowing the system to handle a large number of publishers and subscribers
Implement a heartbeat mechanism to check the liveness of publishers and subscribers.
Compress messages before transmitting to reduce bandwidth usage.
Require explicit acknowledgments from subscribers to ensure they successfully received a message.

