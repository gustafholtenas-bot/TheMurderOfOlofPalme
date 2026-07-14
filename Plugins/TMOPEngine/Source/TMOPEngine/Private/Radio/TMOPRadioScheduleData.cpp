#include "Radio/TMOPRadioScheduleData.h"

int32 UTMOPRadioScheduleData::FindChannelIndex(const FName ChannelId) const
{
    return Channels.IndexOfByPredicate([ChannelId](const FTMOPRadioChannel& Channel)
    {
        return Channel.ChannelId == ChannelId;
    });
}

const FTMOPRadioProgramSegment* UTMOPRadioScheduleData::FindSegment(
    const int32 ChannelIndex, const int32 SecondOfDay) const
{
    if (!Channels.IsValidIndex(ChannelIndex)) return nullptr;
    for (const FTMOPRadioProgramSegment& Segment : Channels[ChannelIndex].Segments)
        if (Segment.ContainsSecond(SecondOfDay)) return &Segment;
    return nullptr;
}
