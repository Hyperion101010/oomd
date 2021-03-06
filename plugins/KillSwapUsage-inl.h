/*
 * Copyright (C) 2018-present, Facebook, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <string>
#include <utility>
#include <vector>

#include "oomd/Log.h"
#include "oomd/include/Types.h"
#include "oomd/util/Fs.h"

namespace Oomd {

template <typename Base>
int KillSwapUsage<Base>::init(
    Engine::MonitoredResources& resources,
    const Engine::PluginArgs& args) {
  if (args.find("cgroup") != args.end()) {
    auto cgroups = Fs::split(args.at("cgroup"), ',');
    resources.insert(cgroups.begin(), cgroups.end());
    cgroups_.insert(cgroups.begin(), cgroups.end());
    cgroup_fs_ =
        (args.find("cgroup_fs") != args.end() ? args.at("cgroup_fs")
                                              : kCgroupFs);
  } else {
    OLOG << "Argument=cgroup not present";
    return 1;
  }

  if (args.find("post_action_delay") != args.end()) {
    int val = std::stoi(args.at("post_action_delay"));

    if (val < 0) {
      OLOG << "Argument=post_action_delay must be non-negative";
      return 1;
    }

    post_action_delay_ = val;
  }

  if (args.find("dry") != args.end()) {
    const std::string& val = args.at("dry");

    if (val == "true" || val == "True" || val == "1") {
      dry_ = true;
    }
  }

  // Success
  return 0;
}

template <typename Base>
Engine::PluginRet KillSwapUsage<Base>::run(OomdContext& ctx) {
  bool ret = tryToKillSomething(ctx);

  if (ret) {
    std::this_thread::sleep_for(std::chrono::seconds(post_action_delay_));
    return Engine::PluginRet::STOP;
  } else {
    return Engine::PluginRet::CONTINUE;
  }
}

template <typename Base>
bool KillSwapUsage<Base>::tryToKillSomething(OomdContext& ctx) {
  auto swap_sorted = ctx.reverseSort(
      [](const CgroupContext& cgroup_ctx) { return cgroup_ctx.swap_usage; });
  OomdContext::dumpOomdContext(swap_sorted);
  Base::removeSiblingCgroups(cgroups_, swap_sorted);
  OLOG << "Removed sibling cgroups";
  OomdContext::dumpOomdContext(swap_sorted);

  for (const auto& state_pair : swap_sorted) {
    // If our target isn't using any swap, killing it won't free up any either
    if (!state_pair.second.swap_usage) {
      continue;
    }

    OLOG << "Picked \"" << state_pair.first << "\" ("
         << state_pair.second.current_usage / 1024 / 1024
         << "MB) based on swap usage at "
         << state_pair.second.swap_usage / 1024 / 1024 << "MB";
    if (Base::tryToKillCgroup(
            cgroup_fs_ + "/" + state_pair.first, true, dry_)) {
      Base::logKill(state_pair.first, state_pair.second, dry_);
      return true;
    }
  }

  return false;
}

} // namespace Oomd
