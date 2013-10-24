#ifndef KNIGHT_COLOR_FORMATTER_H_
#define KNIGHT_COLOR_FORMATTER_H_

#include <logog.hpp>

namespace logog {

class ColorFormatter : public FormatterGCC {
  LOGOG_STRING &Format(const Topic &topic, const Target &target)
  {
    TOPIC_FLAGS flags;
    flags = GetTopicFlags(topic);

    m_sMessageBuffer.clear();

    // Bold first part of message
    m_sMessageBuffer.append("\x1b[1m");

    if (flags & TOPIC_FILE_NAME_FLAG)
    {
      m_sMessageBuffer.append(topic.FileName());
      m_sMessageBuffer.append(':');
    }

    if (flags & TOPIC_LINE_NUMBER_FLAG)
    {
      m_sIntBuffer.assign(topic.LineNumber());
      m_sMessageBuffer.append(m_sIntBuffer);

      m_sMessageBuffer.append(LOGOG_CONST_STRING(": "));
    }

    RenderTimeOfDay();

    // Color topic level
    auto level = topic.Level();
    if (level <= LOGOG_LEVEL_ERROR) {
      m_sMessageBuffer.append("\x1b[31m");
    } else if (level <=  LOGOG_LEVEL_WARN3) {
      m_sMessageBuffer.append("\x1b[35m");
    } else if (level <= LOGOG_LEVEL_INFO) {
      m_sMessageBuffer.append("\x1b[30m");
    } else {
      m_sMessageBuffer.append("\x1b[32m");
    }

    if (flags & TOPIC_LEVEL_FLAG)
    {
      m_sMessageBuffer.append(ErrorDescription(topic.Level()));
      m_sMessageBuffer.append(LOGOG_CONST_STRING(": "));
    }

    // Remove bold and color for user text
    m_sMessageBuffer.append("\x1b[0m");

    if (flags & TOPIC_GROUP_FLAG)
    {
      m_sMessageBuffer.append(LOGOG_CONST_STRING("{"));
      m_sMessageBuffer.append(topic.Group());
      m_sMessageBuffer.append(LOGOG_CONST_STRING("} "));
    }

    if (flags & TOPIC_CATEGORY_FLAG)
    {
      m_sMessageBuffer.append(LOGOG_CONST_STRING("["));
      m_sMessageBuffer.append(topic.Category());
      m_sMessageBuffer.append(LOGOG_CONST_STRING("] "));
    }

    if (flags & TOPIC_MESSAGE_FLAG)
    {
      m_sMessageBuffer.append(topic.Message());
      m_sMessageBuffer.append((LOGOG_CHAR)'\n');
    }

    if (target.GetNullTerminatesStrings())
      m_sMessageBuffer.append((LOGOG_CHAR)NULL);

    return m_sMessageBuffer;
  }
};

}; // namespace logog

#endif // KNIGHT_COLOR_FORMATTER_H_
