# pub-sub
A PUB-SUB library in C/C++

Functionalities :
====================
Coordinator can send msgs to Subs via multiple IPCs : msgQ, Net Sockets ( UDP ), callbacks. Y
Subscribers which are in same Virtual Address Space ( Different threads of same process ) must be able to commuincate via Callbacks. Y
Subscribers which are on same machine must be able to communicate via shared memory Or Unix-Sockts Or msgQs Y
Publishers and Subscribers can Dynamically join and leave Y
Coordinator must be able recover from process restarts Y
Coordinator must support message prioritization Y
When new subscriber Join, he can request bulk transfer of messages from Coordinator Y
Provide real-time dashboards showing active publishers, subscribers, message rates Y
Log all events for auditing and debugging. Y
Scale the distributor by implementing clustering, allowing the system to handle a large number of publishers and subscribers N
Implement a heartbeat mechanism to check the liveness of publishers and subscribers. N
Compress messages before transmitting to reduce bandwidth usage. N
Require explicit acknowledgments from subscribers to ensure they successfully received a message. N
Making Coordinator Multithreaded, pinning the msg distributor's thread to CPU Y

