NWNX Connect 1.0 Pre-Release
----

NWNX Connect is a pair of NWN protocol extension plugins (one for the server, one for the client).
This is a PoC, the plan is to export the handler hooks to other plugins, allowing custom client-server communication.
Right now its only function is to load HAKs and TLK on the client after successful connection to the server.

Supported configuration: Server running on Linux with NWNX 2.8, Client running on Windows with NWNCX 0.2.1.
To install, just copy nwnx_connect.so to the server folder, nwncx_connect.dll to the client folder. No further configuration is required.
This is a pre-release version, please test it and give a feedback in this topic:
http://social.bioware.com/forum/1/topic/199/index/4932567/LP


(c) by virusman, 2012