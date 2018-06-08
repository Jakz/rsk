#include "tbx/base/common.h"

namespace formats
{
  namespace cue
  {
    using integral = u16;
    
    enum class FileMode
    {
      WAVE,
      MP3,
      AIFF,
      BINARY,
      MOTOROLA
    };
    
    enum class TrackFlag
    {
      DCP  = 0x01,
      _4CH = 0x02,
      PRE  = 0x04,
      SCMS = 0x08
    };
    
    enum class TrackType
    {
      AUDIO,
      CDG,
      MODE1_2048,
      MODE1_2352,
      
      MODE2_2336,
      MODE2_2352,
      
      CDI_2336,
      CDI_2352
    };
    
    struct index_point
    {
      integral minutes;
      integral seconds;
      integral frames;
    };
    
    struct Command { };
    
    struct text_command : public Command { std::string value; };
    
    //INDEX 01 12:34:00
    struct IndexCommand : public Command
    {
      integral index; // 0-99
      index_point time;
    };
    
    //FILE "path" BINARY
    //FILE path AUDIO
    struct File
    {
      std::string path;
      FileMode mode;
    };
    
    //FLAGS DCP PRE
    struct FlagCommand : public Command
    {
      
    };
    
    //POSTGAP 12:34:00
    //PREGAM 56:78:90
    struct GapCommand : public Command
    {
      enum class Type { PRE, POST };
      Type type;
      index_point time;
    };
    
    //TRACK 01 MODE1/2048
    //TRACK 04 AUDIO
    struct TrackCommand : public Command
    {
      integral index; // 1-99
      TrackType type;
    };
    
    //CATALOG 123456789
    using CatalogCommand = text_command;
    //CDTEXTFILED path
    using CdTextFileCommand = text_command;
    //ISRC ABCDE12356
    using IsrcCommand = text_command;
    //PERFORMER name
    using PerformerCommand = text_command;
    //REM comment
    using CommentCommand = text_command;
    //SONGWRITER writer
    using SongWriterCommand = text_command;
  
  
    struct Environment
    {
      File file;
    };
    
    struct Cue
    {
      std::vector<std::unique_ptr<Command>> commands;
    };
    
    struct istring
    {
      std::string data;
      
      bool operator==(const std::string& other) const { return strings::caseInsensitiveEqual(data, other); }
      
      istring(const std::string& data) : data(data) { }
    };
    
    //TODO: better message
#define EXPECT_MORE_TOKENS(x, t) if (i+x >= count) throw exceptions::file_format_error("cue format error: " t);

    // https://wiki.hydrogenaud.io/index.php?title=Cue_sheet
    // http://www.totalsonic.net/cuesheetsyntax.htm
    
    using namespace std;
    Cue parse(Environment& env, const vector<istring>& tokens)
    {
      const size_t count = tokens.size();
      for (size_t i = 0; i < count; ++i)
      {
        const auto& t = tokens[i];
        
        if (t == "file")
        {
          EXPECT_MORE_TOKENS(2, "required 2 more tokens for FILE command");
          
          const auto& tmode = tokens[i+2];
          FileMode mode;
          
          if (tmode == "binary")
            mode = FileMode::BINARY;
          else if (tmode == "aiff")
            mode = FileMode::AIFF;
          else if (tmode == "motorola")
            mode = FileMode::MOTOROLA;
          else if (tmode == "mp3")
            mode = FileMode::MP3;
          else if (tmode == "wave")
            mode = FileMode::WAVE;
          else
            throw exceptions::file_format_error(fmt::sprintf("cue format error: unknown FILE mode '%s'", tmode.data));
          
        }
      }
      
      
      return Cue();
    }
    
    vector<istring> tokenize(const std::string& line)
    {
      string t;
      vector<istring> tokens;
      bool quote = false;
      
      for (const auto c : line)
      {
        if (std::isspace(c) && !quote)
        {
          if (!t.empty())
          {
            tokens.push_back(t);
            t.clear();
          }
          continue;
        }
        else if (c == '"')
        {
          quote = !quote;
        }
        else
          t += c;
      }
      
      return tokens;
    }
    
  }
}
