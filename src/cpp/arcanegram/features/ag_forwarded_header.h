#pragma once

#include "base/basic_types.h"
#include "ui/text/text_entity.h"

class HistoryItem;

namespace Settings::Builder {
class SectionBuilder;
} // namespace Settings::Builder

namespace Arcanegram::ForwardedHeader {

void Append(
    TextWithEntities &phrase,
    not_null<const HistoryItem*> item,
    TimeId originalDate);
void Setup(::Settings::Builder::SectionBuilder &builder);
void Init();

} // namespace Arcanegram::ForwardedHeader
