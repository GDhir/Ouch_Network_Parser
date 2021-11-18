#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <unordered_map>

using namespace std;

enum MessageType { accepted = 0, systemEvent = 1, replaced = 2, cancelled = 3, executed = 4 };

unordered_map<char, int> expectedSize{ {'S', 13}, {'A', 68}, {'U', 82}, {'E', 43}, {'C', 31} };
uint32_t swap_uint32( uint val );

class Stream {

    int accepted{0};
    int systemEvent{0};
    int replaced{0};
    int cancelled{0};
    int executed{0};
    int executedShares{0};

    public:

        void updateEvent( char messageChar ) {

            switch( messageChar ) {

                case 'A': accepted += 1;
                case 'S': systemEvent += 1;
                case 'U': replaced += 1;
                case 'C': cancelled += 1;
                case 'E': executed += 1;

            } 


        }

        void printStream( const int& streamID ) {

            cout << "Stream \t" << streamID << "\n";
            cout << "Accepted \t" << accepted << "\n";
            cout << "Replaced \t" << replaced << "\n";
            cout << "System Event \t" << systemEvent << "\n";
            cout << "Cancelled \t" << cancelled << "\n";
            cout << "Executed \t" << executed << "\n";
            cout << "Executed Shares \t" << executedShares << "\n";
        
        }

        void updateExecutedShares(int shares) {

            executedShares += swap_uint32( shares );

        }

};

struct PacketStatus {

    int countUpdated{0};
    char type;

};

unordered_map<int, PacketStatus > partialPacket;
unordered_map<int, Stream> nPackets;

//! Byte swap unsigned short
uint16_t swap_uint16( uint16_t val ) 
{
    return (val << 8) | (val >> 8 );
}

//! Byte swap unsigned int
uint32_t swap_uint32( uint val )
{
    val = ((val << 8) & 0xFF00FF00 ) | ((val >> 8) & 0xFF00FF ); 
    return (val << 16) | (val >> 16);
}

void getPacketHeader( uint* buffer, FILE* ifs ) {

    // function used to read the contents of file
        fread(&buffer[0], 2, 1, ifs);
        fread(&buffer[1], 4, 1, ifs );

        buffer[0] = swap_uint32( buffer[0] );
        buffer[1] = swap_uint32( buffer[1] );

        //cout << buffer[0] << "\n" << buffer[1];

}

void parsePacket( FILE* ifs, uint* buffer, int packetSize, int sID  ) {

    if( partialPacket.find( sID ) != partialPacket.end() ) {

        if( partialPacket[ sID ].countUpdated <= 3 ) {

            if( partialPacket[ sID ].countUpdated + packetSize > 3 ) {

                char val;
                fseek( ifs, 3 - partialPacket[sID].countUpdated, SEEK_CUR );
                fread( &val, 1, 1, ifs );
                nPackets[ sID ].updateEvent( val );
                partialPacket[ sID ].type = val;  

                if( val == 'E' ) {
                    partialPacket[sID].type = 'E';

                    if( partialPacket[sID].countUpdated + packetSize > 26 ) {

                        fseek( ifs, 22, SEEK_CUR );
                        uint shares;
                        fread( &shares, 4, 1, ifs );
                        nPackets[ sID ].updateExecutedShares( shares );
                        fseek( ifs, packetSize - ( 4 - partialPacket[sID].countUpdated + 26 ), SEEK_CUR );

                        if( partialPacket[sID].countUpdated + packetSize == expectedSize['E'] ) {

                            partialPacket.erase(sID);

                        }
                        else {

                            partialPacket[sID].countUpdated += packetSize;

                        }

                    }
                    else {

                        fseek( ifs, packetSize - ( 4 - partialPacket[sID].countUpdated), SEEK_CUR );
                        partialPacket[sID].countUpdated += packetSize;                        

                    }

                }
                else {

                    auto type = partialPacket[sID].type;
                    fseek( ifs, packetSize - ( 4 - partialPacket[sID].countUpdated ) ,SEEK_CUR );

                    if( partialPacket[sID].countUpdated + packetSize == expectedSize[ val ] ) {

                        partialPacket.erase(sID);

                    }
                    else {

                        partialPacket[sID].countUpdated += packetSize;

                    }

                    

                }

            }
            else {
                
                fseek( ifs, packetSize, SEEK_CUR );
                partialPacket[sID].countUpdated += packetSize;

            }

        }
        else {

            if( partialPacket[sID].type == 'E' ) {

                if( partialPacket[sID].countUpdated < 26 ) {
                    
                    if( partialPacket[sID].countUpdated + packetSize > 26 ) {
                        
                        fseek( ifs, 26 - partialPacket[sID].countUpdated, SEEK_CUR );
                        uint shares;
                        fread( &shares, 4, 1, ifs );
                        nPackets[sID].updateExecutedShares( shares );
                        
                        fseek( ifs, packetSize - ( 30 - partialPacket[sID].countUpdated ), SEEK_CUR );

                        if( partialPacket[sID].countUpdated + packetSize == expectedSize['E'] ) {

                            partialPacket.erase( sID );

                        }
                        else {

                            partialPacket[sID].countUpdated += packetSize;

                        }

                    }
                    else {

                        fseek( ifs, packetSize, SEEK_CUR );
                        partialPacket[sID].countUpdated += packetSize;

                    }

                }
                else {
                    
                    fseek( ifs, packetSize, SEEK_CUR );

                    if( partialPacket[sID].countUpdated + packetSize == expectedSize[ 'E' ] ) {

                        partialPacket.erase(sID);

                    }
                    else {

                        partialPacket[sID].countUpdated += packetSize;

                    }

                }

            }
            else {

                fseek( ifs, packetSize, SEEK_CUR );

                if( partialPacket[sID].countUpdated + packetSize == expectedSize[ partialPacket[sID].type ] ) {

                    partialPacket.erase(sID);

                }
                else {

                    partialPacket[sID].countUpdated += packetSize;

                }

            }

        }
    }
    else {

        if( packetSize <= 3 ) {

            partialPacket[sID] = PacketStatus{ packetSize, '/' };
            fseek( ifs, packetSize, SEEK_CUR );

        }
        else {

            char val;
            fseek( ifs, 3, SEEK_CUR );
            fread( &val, 1, 1, ifs );
            nPackets[ sID ].updateEvent( val );            

            if( val == 'E' ) {

                if( packetSize > 26 ) {

                    fseek( ifs, 22, SEEK_CUR );
                    uint shares;
                    fread( &shares, 4, 1, ifs );
                    fseek( ifs, packetSize - 30, SEEK_CUR );

                    nPackets[ sID ].updateExecutedShares( shares );

                    if( packetSize != expectedSize['E'] ) {

                        partialPacket[sID] = { packetSize, 'E' };
                    }

                }
                else {

                    fseek( ifs, packetSize - 4, SEEK_CUR );
                    partialPacket[sID] = { packetSize, 'E' };

                }

            }
            else {

                fseek( ifs, packetSize - 4, SEEK_CUR );

                if( packetSize != expectedSize[val] ) {

                    partialPacket[sID] = { packetSize, val };

                }

            }

        }

    }

}

void parsePackets( FILE* ifs ) {

    uint buffer[2];

    while( ftell(ifs) < 5702889 ) {

        getPacketHeader( buffer, ifs );
        parsePacket( ifs, buffer, buffer[1], buffer[0] );

        buffer[0] = buffer[1] = 0;

    }


}

void printOutput() {

    for( auto& streamPair: nPackets ) {

        streamPair.second.printStream( streamPair.first );
        cout << endl;

    }

}


int main() {

    FILE* ifs = fopen( "OUCHLMM2.incoming.packets", "rb" );

    //fseek( ifs, 0, SEEK_END );
    //auto sz = ftell(ifs);
    // Define a vector to store the strings received from input
    uint buffer[2];
    
    

    parsePackets( ifs );
    fclose(ifs);

    printOutput();

    return 0;
}