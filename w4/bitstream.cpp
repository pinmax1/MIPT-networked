#include"bitstream.h"
#include <iostream>
// enum MessageType : uint8_t
// {
//   E_CLIENT_TO_SERVER_JOIN = 0,
//   E_SERVER_TO_CLIENT_NEW_ENTITY,
//   E_SERVER_TO_CLIENT_SET_CONTROLLED_ENTITY,
//   E_CLIENT_TO_SERVER_STATE,
//   E_SERVER_TO_CLIENT_SNAPSHOT
// };



// int main() {
//   uint8_t* ptr = new uint8_t[sizeof(std::string)];
//   for(int i = 0; i < 5; ++i) {
//     ptr[i] = (uint8_t)i;
//   }
//   std::string a = "dada";
//   Bitstream bs;
//   bs.Write(a);
//   memcpy(ptr, bs.Data(), bs.Size());
//   Bitstream bs2(ptr, sizeof(std::string));
//   std::string b;
//   bs2.Read(b);
//   std::cout << b;
//   delete &a;
// }