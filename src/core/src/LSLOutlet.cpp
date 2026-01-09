#include "lsltemplate/LSLOutlet.hpp"

namespace lsltemplate {

LSLOutlet::LSLOutlet(const DeviceInfo& info)
    : info_(info)
{
    // Determine channel format (using float32 for this template)
    lsl::channel_format_t format = lsl::cf_float32;

    // Create stream info
    lsl::stream_info stream_info(
        info.name,
        info.type,
        info.channel_count,
        info.sample_rate,
        format,
        info.source_id
    );

    // Add metadata (customize for your application)
    lsl::xml_element desc = stream_info.desc();
    desc.append_child_value("manufacturer", "LSL Template");

    // Add channel descriptions
    lsl::xml_element channels = desc.append_child("channels");
    for (int i = 0; i < info.channel_count; ++i) {
        lsl::xml_element ch = channels.append_child("channel");
        ch.append_child_value("label", "Ch" + std::to_string(i + 1));
        ch.append_child_value("unit", "arbitrary");
        ch.append_child_value("type", info.type);
    }

    // Create the outlet
    outlet_ = std::make_unique<lsl::stream_outlet>(stream_info);
}

LSLOutlet::~LSLOutlet() = default;

void LSLOutlet::pushChunk(const std::vector<float>& data) {
    if (outlet_ && !data.empty()) {
        outlet_->push_chunk_multiplexed(data);
    }
}

void LSLOutlet::pushSample(const std::vector<float>& sample) {
    if (outlet_ && !sample.empty()) {
        outlet_->push_sample(sample);
    }
}

std::string LSLOutlet::getStreamName() const {
    return info_.name;
}

bool LSLOutlet::hasConsumers() const {
    return outlet_ && outlet_->have_consumers();
}

} // namespace lsltemplate
