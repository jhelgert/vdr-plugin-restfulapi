#include "remote.h"

void RemoteResponder::reply(std::ostream& out, cxxtools::http::Request& request, cxxtools::http::Reply& reply)
{
  if (request.method() != "POST") {
     reply.httpReturn(403, "Only POST method is support by the remote control");
     return;
  }

  std::string params = getRestParams((std::string)"/remote", request.url());
  std::string key = getStringParam(params, 0);

  if (key.length() == 0) {
     reply.httpReturn(404, "Please add a key to the parameter list, see API-file for more details.");
     return;
  }

  if (!keyPairList->hitKey(key.c_str())) {
     reply.httpReturn(404, "Remote Control does not support the requested key.");
  }
}

KeyPairList::KeyPairList()
{
  append( "up" , 		kUp 		);
  append( "down" , 		kDown 		);
  append( "menu" , 		kMenu 		);
  append( "ok" , 		kOk 		);
  append( "back" , 		kBack 		);
  append( "left" , 		kLeft 		);
  append( "right" , 		kRight 		);
  append( "red" , 		kRed 		);
  append( "green" , 		kGreen 		);
  append( "yellow" , 		kYellow 	);
  append( "blue" , 		kBlue 		);
  append( "0" , 		k0 		);
  append( "1" , 		k1 		);
  append( "2" , 		k2 		);
  append( "3" , 		k3 		);
  append( "4" , 		k4 		);
  append( "5" , 		k5 		);
  append( "6" , 		k6 		);
  append( "7" , 		k7 		);
  append( "8" , 		k8 		);
  append( "9" , 		k9 		);
  append( "info" , 		kInfo 		);
  append( "play" , 		kPlay 		);
  append( "pause" , 		kPause 		);
  append( "stop" , 		kStop 		);
  append( "record" , 		kRecord 	);
  append( "fastfwd" , 		kFastFwd 	);
  append( "fastrew" , 		kFastRew 	);
  append( "next" , 		kNext 		);
  append( "prev" , 		kPrev 		);
  append( "power" , 		kPower 		);
  append( "chanup" , 		kChanUp 	);
  append( "chandn" , 		kChanDn 	);
  append( "volup" , 		kVolUp 		);
  append( "voldn" , 		kVolDn 		);
  append( "mute" , 		kMute 		);
  append( "audio" , 		kAudio 		);
  append( "subtitles" , 	kSubtitles 	);
  append( "schedule" ,		kSchedule 	);
  append( "channels" ,		kChannels 	);
  append( "timers" , 		kTimers 	);
  append( "recordings" , 	kRecordings 	);
  append( "setup" , 		kSetup 		);
  append( "commands" ,		kCommands 	);
  append( "user0" , 		kUser0 		);
  append( "user1" , 		kUser1 		);
  append( "user2" , 		kUser2 		);
  append( "user3" , 		kUser3 		);
  append( "user4" , 		kUser4 		);
  append( "user5" , 		kUser5 		);
  append( "user6" , 		kUser6 		);
  append( "user7" , 		kUser7 		);
  append( "user8" , 		kUser8 		);
  append( "user9" , 		kUser9 		);
  append( "none" , 		kNone 		);
  append( "kbd" , 		kKbd 		);
}

KeyPairList::~KeyPairList()
{

}

bool KeyPairList::hitKey(std::string key)
{ 
  for (int i=0;i<(int)key.length();i++) {
    key[i] = std::tolower(key[i]);
  }
  
  for (int i=0;i<(int)keys.size();i++)
  {
    if (std::string(keys[i].str) == key) {
       cRemote::Put(keys[i].key);
       return true;
    }
  }
  return false;
}

void KeyPairList::append(const char* str, eKeys key)
{
  eKeyPair keyPair = { str, key };
  keys.push_back(keyPair);
}