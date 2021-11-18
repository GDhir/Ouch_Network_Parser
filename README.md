# Ouch_Network_Parser
Repo displays a demo of parsing Ouch Protocol Data from the NASDAQ exchange embedded within a binary file.

A packet capture file is provided in a proprietary format with OUCH protocol packets strewn across multiple TCP streams. 
The program counts the total number of each OUCH Message Type across each stream, and for the Executed Message sums the ExecutedShares field across each stream, 
and displays them as output at the end of the program.

## Packets
The packets that are captured contain within them OUCH protocol message streams. Each packet either contains the full OUCH protocol message or a partial OUCH 
protocol message that is completed by a subsequent packet that belongs to the same stream. The program parses the network data and stores each partial OUCH protocol
message within a hashmap. As soon as the protocol message is completed by a subsequent packet from the same TCP stream, the protocol data is erased from the hashmap.

The final data corresponding to the total number of OUCH Message Type across each stream and the total executed shares is captured within a separate hashmap. 

## OUCH Protocol
The OUCH protocol messages in the capture consist of following message types. All the integer fields in the capture are Big Endian.

![alt text](https://github.com/GDhir/Ouch_Network_Parser/blob/master/Packet_Info_IMG1.png)

![alt text](https://github.com/GDhir/Ouch_Network_Parser/blob/master/Packet_Info_IMG2.png)

![alt text](https://github.com/GDhir/Ouch_Network_Parser/blob/master/Packet_Info_IMG3.png)

![alt text](https://github.com/GDhir/Ouch_Network_Parser/blob/master/Packet_Info_IMG4.png)
