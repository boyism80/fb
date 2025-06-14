#ifndef __INDICATOR_H__
#define __INDICATOR_H__

#include <indicators/block_progress_bar.hpp>
#include <indicators/cursor_control.hpp>
#include <indicators/progress_bar.hpp>

namespace fb {

class LoadProgressBar :
#ifndef _WIN32
    public indicators::BlockProgressBar
#else
    public indicators::ProgressBar
#endif
{
public:
    LoadProgressBar(const std::string& prefix, const std::string& postfix)
#ifndef _WIN32
        :
        indicators::BlockProgressBar(indicators::option::BarWidth{80},
                                     indicators::option::Start{"["},
                                     indicators::option::End{"]"},
                                     indicators::option::ForegroundColor{indicators::Color::white},
                                     indicators::option::ShowPercentage{true},
                                     indicators::option::PrefixText{prefix},
                                     indicators::option::PostfixText{postfix},
                                     indicators::option::FontStyles{std::vector<indicators::FontStyle> {
                                         indicators::FontStyle::bold
                                     }})
#else
        :
        indicators::ProgressBar(indicators::option::BarWidth{50},
                                indicators::option::Start{"["},
                                indicators::option::Fill{"-"},
                                indicators::option::Lead{"-"},
                                indicators::option::Remainder{" "},
                                indicators::option::End{"]"},
                                indicators::option::PrefixText{prefix},
                                indicators::option::PostfixText{postfix},
                                indicators::option::ForegroundColor{indicators::Color::white},
                                indicators::option::ShowPercentage{true})
#endif
    { }
};

} // namespace fb

#endif