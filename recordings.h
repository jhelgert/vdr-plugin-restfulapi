#include <cxxtools/http/request.h>
#include <cxxtools/http/reply.h>
#include <cxxtools/http/responder.h>
#include <cxxtools/arg.h>
#include <cxxtools/jsonserializer.h>
#include <cxxtools/serializationinfo.h>
#include <cxxtools/utf8codec.h>
#include "tools.h"

#include "vdr/recording.h"

class RecordingsResponder : public cxxtools::http::Responder
{
  public:
    explicit RecordingsResponder(cxxtools::http::Service& service)
      : cxxtools::http::Responder(service)
      { }

    virtual void reply(std::ostream&, cxxtools::http::Request& request, cxxtools::http::Reply& reply);
};

typedef cxxtools::http::CachedService<RecordingsResponder> RecordingsService;

struct SerRecording
{
  cxxtools::String Name;
  cxxtools::String FileName;
  bool IsNew;
  bool IsEdited;
  bool IsPesRecording;
};

struct SerRecordings
{
  std::vector < struct SerRecording > recording;
};

void operator<<= (cxxtools::SerializationInfo& si, const SerRecording& p);
void operator<<= (cxxtools::SerializationInfo& si, const SerRecordings& p);

class RecordingList
{
  protected:
    std::ostream* out;
  public:
    RecordingList(std::ostream* _out) { out = _out; };
    ~RecordingList() { };
    virtual void addRecording(cRecording* recording) { };
};

class HtmlRecordingList : RecordingList
{
  public:
    HtmlRecordingList(std::ostream* _out);
    ~HtmlRecordingList();
    virtual void addRecording(cRecording* recording);
};

class JsonRecordingList : RecordingList
{
  private:
    std::vector < struct SerRecording > serRecordings;
  public:
    JsonRecordingList(std::ostream* _out);
    ~JsonRecordingList();
    virtual void addRecording(cRecording* recording);
};