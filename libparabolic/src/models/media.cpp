#include "models/media.h"
#include <libnick/helpers/stringhelpers.h>

using namespace Nickvision::Helpers;

namespace Nickvision::TubeConverter::Shared::Models
{
    Media::Media(boost::json::object info, bool includeAutoGeneratedSubtitles, VideoCodec preferredVideoCodec)
        : m_timeFrame{ std::chrono::seconds(0), std::chrono::seconds(0) }
    {
        //Parse base information
        if(info.contains("is_part_of_playlist"))
        {
            m_url = info.contains("url") ? (info["url"].is_string() ? info["url"].as_string() : "") : (info["webpage_url"].is_string() ? info["webpage_url"].as_string().c_str() : "");
        }
        else
        {
            m_url = info.contains("webpage_url") ? (info["webpage_url"].is_string() ? info["webpage_url"].as_string() : "") : (info["url"].is_string() ? info["url"].as_string() : "");
        }
        m_title = info["title"].is_string() ? info["title"].as_string() : "Media";
        m_title = StringHelpers::normalizeForFilename(m_title, info["limit_characters"].is_bool() ? info["limit_characters"].as_bool() : false);
        if(info.contains("duration"))
        {
            m_timeFrame = { std::chrono::seconds(0), std::chrono::seconds{ info["duration"].is_double() ? static_cast<int>(info["duration"].as_double()) : (info["duration"].is_int64() ? static_cast<int>(info["duration"].as_int64()) : 0) } };
        }
        //Parse formats
        if(info.contains("formats") && info["formats"].is_array())
        {
            for(const boost::json::value& format : info["formats"].as_array())
            {
                if(!format.is_object())
                {
                    continue;
                }
                Format f{ format.as_object() };
                if(f.getType() != MediaType::Image)
                {
                    if(f.getVideoCodec() && preferredVideoCodec != VideoCodec::Any && f.getVideoCodec().value() != preferredVideoCodec)
                    {
                        continue;
                    }
                    m_formats.push_back(f);
                }
            }
        }
        //Parse automatic subtitles
        if(includeAutoGeneratedSubtitles && info.contains("automatic_captions") && info["automatic_captions"].is_object())
        {
            for(const boost::json::key_value_pair& caption : info["automatic_captions"].as_object())
            {
                m_subtitles.push_back({ caption.key(), true });
            }
        }
        //Parse subtitles
        if(info.contains("subtitles") && info["subtitles"].is_object())
        {
            for(const boost::json::key_value_pair& subtitle : info["subtitles"].as_object())
            {
                if(subtitle.key() != "live_chat")
                {
                    m_subtitles.push_back({ subtitle.key(), false });
                }
            }
        }
        std::sort(m_subtitles.begin(), m_subtitles.end());
        //Type
        m_type = MediaType::Audio;
        for(const Format& format : m_formats)
        {
            if(format.getType() == MediaType::Video)
            {
                m_type = MediaType::Video;
                break;
            }
        }
    }

    const std::string& Media::getUrl() const
    {
        return m_url;
    }

    const std::string& Media::getTitle() const
    {
        return m_title;
    }

    MediaType Media::getType() const
    {
        return m_type;
    }

    const TimeFrame& Media::getTimeFrame() const
    {
        return m_timeFrame;
    }

    const std::vector<Format>& Media::getFormats() const
    {
        return m_formats;
    }

    const std::vector<SubtitleLanguage>& Media::getSubtitles() const
    {
        return m_subtitles;
    }
}