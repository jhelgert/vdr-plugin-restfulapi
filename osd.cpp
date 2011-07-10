#include "osd.h"

void OsdResponder::reply(std::ostream& out, cxxtools::http::Request& request, cxxtools::http::Reply& reply)
{
  if ( request.method() != "GET" ) {
     reply.httpReturn(403, "Only GET-method is supported!");
     return;
  }
  
  BasicOsd* osd = StatusMonitor::get()->getOsd();

  if ( osd == NULL ) {
     reply.httpReturn(404, "No OSD opened!");
     return;
  }

  QueryHandler q("/osd", request);
  std::string format = "";
  if ( q.isFormat(".json") ) {
     reply.addHeader("Content-Type", "application/json; charset=utf-8");
     format = ".json";
  } else if ( q.isFormat(".html") ) {
     format = ".html"; //will  be required later for implementation
     reply.addHeader("Content-Type", "text/html; charset=utf-8");
     StreamExtension se(&out);
     se.write("To be implemented!");
     return;
  } else if ( q.isFormat(".xml") ) {
     reply.addHeader("Content-Type", "text/xml; charset=utf-8");
     format = ".xml";
  } else {
     reply.httpReturn(403, "Resources are not available for the selected format. (Use: .json, .html or .xml)");
     return;
  }
  
  int start_filter = q.getOptionAsInt("start");
  int limit_filter = q.getOptionAsInt("limit");

  switch(osd->Type())
  {
     case 0x01: printTextOsd(out, (TextOsd*)osd, format, start_filter, limit_filter);
                break;
     case 0x02: { ChannelOsdWrapper* w = new ChannelOsdWrapper(&out);
                  w->print((ChannelOsd*)osd, format);
                  delete w;  }
                break;
     case 0x03: { ProgrammeOsdWrapper* w = new ProgrammeOsdWrapper(&out);
                  w->print((ProgrammeOsd*)osd, format);
                  delete w; }
                break; 
  }
}

void operator<<= (cxxtools::SerializationInfo& si, const SerTextOsdItem& o)
{
  si.addMember("content") <<= o.Content;
  si.addMember("is_selected") <<= o.Selected;
}

void operator<<= (cxxtools::SerializationInfo& si, const SerTextOsd& o)
{
  si.addMember("type") <<= "TextOsd";
  si.addMember("title") <<= o.Title;
  si.addMember("message") <<= o.Message;
  si.addMember("red") <<= o.Red;
  si.addMember("green") <<= o.Green;
  si.addMember("yellow") <<= o.Yellow;
  si.addMember("blue") <<= o.Blue;

  si.addMember("items") <<= o.ItemContainer->items;  
}

void operator<<= (cxxtools::SerializationInfo& si, const SerProgrammeOsd& o)
{
  si.addMember("present_time") <<= o.PresentTime;
  si.addMember("present_title") <<= o.PresentTitle;
  si.addMember("present_subtitle") <<= o.PresentSubtitle;
  si.addMember("following_time") <<= o.FollowingTime;
  si.addMember("following_title") <<= o.FollowingTitle;
  si.addMember("following_subtitle") <<= o.FollowingSubtitle;
}

void OsdResponder::printTextOsd(std::ostream& out, TextOsd* osd, std::string format, int start_filter, int limit_filter)
{
  TextOsdList* osdList = NULL;

  if ( format == ".json" ) {
     osdList = (TextOsdList*)new JsonTextOsdList(&out);
  } else if ( format == ".xml" ) {
     osdList = (TextOsdList*)new XmlTextOsdList(&out);
  } else if ( format == ".html" ) {
     osdList = (TextOsdList*)new HtmlTextOsdList(&out);
  }

  if ( osdList != NULL ) {
     if (start_filter >= 0 && limit_filter >= 1 ) {
        osdList->activateLimit(start_filter, limit_filter);
     }
     osdList->printTextOsd(osd);
  }
}

// --- XmlTextOsdList --------------------------------------------------------------------------------

void XmlTextOsdList::printTextOsd(TextOsd* osd)
{
  s->writeXmlHeader();
  s->write("<TextOsd xmlns=\"http://www.domain.org/restfulapi/2011/TextOsd-xml\">\n");
  s->write((const char*)cString::sprintf(" <title>%s</title>\n", StringExtension::encodeToXml(osd->Title()).c_str()));
  s->write((const char*)cString::sprintf(" <message>%s</message>\n", StringExtension::encodeToXml(osd->Message()).c_str()));
  s->write((const char*)cString::sprintf(" <red>%s</red>\n", StringExtension::encodeToXml(osd->Red()).c_str()));
  s->write((const char*)cString::sprintf(" <green>%s</green>\n", StringExtension::encodeToXml(osd->Green()).c_str()));
  s->write((const char*)cString::sprintf(" <yellow>%s</yellow>\n", StringExtension::encodeToXml(osd->Yellow()).c_str()));
  s->write((const char*)cString::sprintf(" <blue>%s</blue>\n", StringExtension::encodeToXml(osd->Blue()).c_str()));
  
  std::list<TextOsdItem*>::iterator it;
  std::list<TextOsdItem*> items = osd->GetItems();

  s->write(" <items>\n");
  for( it = items.begin(); it != items.end(); ++it ) {
    if (!filtered()) {
       const char* selected = (*it) == osd->Selected() ? "true" : "false";
       TextOsdItem* item = *it;
       s->write((const char*)cString::sprintf("  <item selected=\"%s\">%s</item>\n", selected, StringExtension::encodeToXml(item->Text()).c_str()));
    }
  }
  s->write(" </items>\n");
  s->write("</TextOsd>\n");
}

// --- JsonTextOsdList -------------------------------------------------------------------------------

void JsonTextOsdList::printTextOsd(TextOsd* textOsd)
{
  SerTextOsd t;

  t.Title = StringExtension::UTF8Decode(textOsd->Title());
  t.Message = StringExtension::UTF8Decode(textOsd->Message());
  t.Red = StringExtension::UTF8Decode(textOsd->Red());
  t.Green = StringExtension::UTF8Decode(textOsd->Green());
  t.Yellow = StringExtension::UTF8Decode(textOsd->Yellow());
  t.Blue = StringExtension::UTF8Decode(textOsd->Blue());

  SerTextOsdItemContainer* itemContainer = new SerTextOsdItemContainer();

  std::list<TextOsdItem*>::iterator it;
  std::list<TextOsdItem*> items = textOsd->GetItems();
  
  for(it = items.begin(); it != items.end(); ++it)
  {
    if (!filtered()) {
       SerTextOsdItem sitem;
       sitem.Content = cxxtools::String(StringExtension::UTF8Decode((*it)->Text()));
       sitem.Selected = (*it) == textOsd->Selected() ? true : false;
       itemContainer->items.push_back(sitem);
    }
  }  
  
  t.ItemContainer = itemContainer;;

  cxxtools::JsonSerializer serializer(*s->getBasicStream());
  serializer.serialize(t, "TextOsd");
  serializer.finish();

  //delete itemsArray;
  delete itemContainer;
}

// --- HtmlTextOsdList -------------------------------------------------------------------------------

void HtmlTextOsdList::printTextOsd(TextOsd* textOsd)
{
  s->writeHtmlHeader();
  //to be implemented
}

// --- ProgrammeOsdWrapper ---------------------------------------------------------------------------

void ProgrammeOsdWrapper::print(ProgrammeOsd* osd, std::string format)
{
  if ( format == ".json" ) {
     printJson(osd);
  } else if ( format == ".html" ) {
     printHtml(osd);
  } else if ( format == ".xml") {
     printXml(osd);
  }
}

void ProgrammeOsdWrapper::printXml(ProgrammeOsd* osd)
{
  s->writeXmlHeader();
  s->write("<ProgrammeOsd xmlns=\"http://www.domain.org/restfulapi/2011/ProgrammeOsd-xml\">\n");
  s->write((const char*)cString::sprintf(" <presenttime>%i</presenttime>\n", (int)osd->PresentTime()));
  s->write((const char*)cString::sprintf(" <presenttitle>%s</presenttitle>\n", StringExtension::encodeToXml(osd->PresentTitle()).c_str()));
  s->write((const char*)cString::sprintf(" <presentsubtitle>%s</presentsubtitle>\n", StringExtension::encodeToXml(osd->PresentSubtitle()).c_str()));
  s->write((const char*)cString::sprintf(" <followingtime>%i</followingtime>\n", (int)osd->FollowingTime()));
  s->write((const char*)cString::sprintf(" <followingtitle>%s</followingtitle>\n", StringExtension::encodeToXml(osd->FollowingTitle()).c_str()));
  s->write((const char*)cString::sprintf(" <followingsubtitle>%s</followingsubtitle>\n", StringExtension::encodeToXml(osd->FollowingSubtitle()).c_str()));
  s->write("</ProgrammeOsd>\n");
}

void ProgrammeOsdWrapper::printJson(ProgrammeOsd* osd)
{
  cxxtools::JsonSerializer serializer(*s->getBasicStream());
  SerProgrammeOsd p;
  p.PresentTime = osd->PresentTime();
  p.PresentTitle = StringExtension::UTF8Decode(osd->PresentTitle());
  p.PresentSubtitle = StringExtension::UTF8Decode(osd->PresentSubtitle());
  p.FollowingTime = osd->FollowingTime();
  p.FollowingTitle = StringExtension::UTF8Decode(osd->FollowingTitle());
  p.FollowingSubtitle = StringExtension::UTF8Decode(osd->FollowingSubtitle());
  serializer.serialize(p, "ProgrammeOsd");
  serializer.finish();
}

void ProgrammeOsdWrapper::printHtml(ProgrammeOsd* osd)
{
  s->writeHtmlHeader();
  //to be implemented
}

// --- ChannelOsdWrapper -----------------------------------------------------------------------------

void ChannelOsdWrapper::print(ChannelOsd* osd, std::string format)
{
  if ( format == ".json" ) {
     printJson(osd);
  } else if ( format == ".html" ) {
     printHtml(osd);
  } else if ( format == ".xml" ) {
     printXml(osd);
  }
}

void ChannelOsdWrapper::printXml(ChannelOsd* osd)
{
  s->writeXmlHeader();
  s->write("<ChannelOsd xmlns=\"http://www.domain.org/restfulapi/2011/ChannelOsd-xml\">\n");
  s->write((const char*)cString::sprintf(" <Text>%s</Text>\n", StringExtension::encodeToXml(osd->Channel()).c_str()));
  s->write("</ChannelOsd>\n");
}

void ChannelOsdWrapper::printJson(ChannelOsd* osd)
{
  cxxtools::JsonSerializer serializer(*s->getBasicStream());
  serializer.serialize(StringExtension::UTF8Decode(osd->Channel()), "ChannelOsd"); 
  serializer.finish();
}

void ChannelOsdWrapper::printHtml(ChannelOsd* osd)
{
  s->writeHtmlHeader();
  //to be implemented
}