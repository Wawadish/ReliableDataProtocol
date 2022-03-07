# ReliableDataProtocol
A reliable data protocol (RDP) built on top of UDP for the GINI Router.

Handles multiple connections, uses UDP to reliably transfer data over a connection. The reliability can be tested by simulating a DDOS attack using GINI.

Install GINI Router and copy the files onto the backend folder.
Use gnc --rdp <host> <port> to connect to an RDP host.
Use gnc [-u] -l <port> to setup an RDP host.
  
This project was written for a Computer Networks assignment (grade: 120%)
