// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/render_target_cache.h"
#include "impeller/renderer/render_target.h"

namespace impeller {

RenderTargetCache::RenderTargetCache(std::shared_ptr<Allocator> allocator)
    : RenderTargetAllocator(std::move(allocator)) {}

void RenderTargetCache::Start() {
  for (auto& td : render_target_data_) {
    td.used_this_frame = false;
  }
}

void RenderTargetCache::End() {
  std::vector<RenderTargetData> retain;

  for (const auto& td : render_target_data_) {
    if (td.used_this_frame) {
      retain.push_back(td);
    }
  }
  render_target_data_.swap(retain);
}

RenderTarget RenderTargetCache::CreateOffscreen(
    const Context& context,
    ISize size,
    int mip_count,
    const std::string& label,
    RenderTarget::AttachmentConfig color_attachment_config,
    std::optional<RenderTarget::AttachmentConfig> stencil_attachment_config) {
  auto config = RenderTargetConfig{
      .size = size,
      .mip_count = static_cast<size_t>(mip_count),
      .has_msaa = false,
      .has_depth_stencil = stencil_attachment_config.has_value(),
  };
  for (auto& render_target_data : render_target_data_) {
    const auto other_config = render_target_data.config;
    if (!render_target_data.used_this_frame && other_config == config) {
      render_target_data.used_this_frame = true;
      return render_target_data.render_target;
    }
  }
  RenderTarget created_target = RenderTargetAllocator::CreateOffscreen(
      context, size, mip_count, label, color_attachment_config,
      stencil_attachment_config);
  if (!created_target.IsValid()) {
    return created_target;
  }
  render_target_data_.push_back(
      RenderTargetData{.used_this_frame = true,
                       .config = config,
                       .render_target = created_target});
  return created_target;
}

RenderTarget RenderTargetCache::CreateOffscreenMSAA(
    const Context& context,
    ISize size,
    int mip_count,
    const std::string& label,
    RenderTarget::AttachmentConfigMSAA color_attachment_config,
    std::optional<RenderTarget::AttachmentConfig> stencil_attachment_config) {
  auto config = RenderTargetConfig{
      .size = size,
      .mip_count = static_cast<size_t>(mip_count),
      .has_msaa = true,
      .has_depth_stencil = stencil_attachment_config.has_value(),
  };
  for (auto& render_target_data : render_target_data_) {
    const auto other_config = render_target_data.config;
    if (!render_target_data.used_this_frame && other_config == config) {
      render_target_data.used_this_frame = true;
      return render_target_data.render_target;
    }
  }
  RenderTarget created_target = RenderTargetAllocator::CreateOffscreenMSAA(
      context, size, mip_count, label, color_attachment_config,
      stencil_attachment_config);
  if (!created_target.IsValid()) {
    return created_target;
  }
  render_target_data_.push_back(
      RenderTargetData{.used_this_frame = true,
                       .config = config,
                       .render_target = created_target});
  return created_target;
}

size_t RenderTargetCache::CachedTextureCount() const {
  return render_target_data_.size();
}

}  // namespace impeller
