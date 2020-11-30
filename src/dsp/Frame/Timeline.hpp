#pragma once

#include "definitions.hpp"
#include "Section.hpp"
#include <vector>

namespace myrisa {
namespace dsp {
namespace frame {

/**
   Top level structure for manifestations.
   It is a sequence of Sections.
*/
struct Timeline {
  // TODO render

  std::vector<Section*> sections;
  // maps beat #'s to <section i, section beat>
  std::vector<std::pair<int, int>> beat_map;
  int n_beats = 0;

  inline void pushBackSection(Section* new_section) {

  }

  inline bool sectionExistsForBeat(int beat) {
    return beat < n_beats;
  }

  inline void assertInvariants() {
    assert(sections.size() == beat_map.size());
    if (0 < n_beats) {
      assert(sectionExistsForBeat(n_beats-1));
    }
  }

  inline std::pair<Section*, int> getSectionAndSectionBeatAtBeat(int beat) {
    if (n_beats <= beat) {
      return std::pair<Section*, int>(nullptr, 0);
    }

    Section* section_at_beat = sections[beat_map[beat].first];
    int section_beat = beat_map[beat].second;
    return std::pair<Section*, int>(section_at_beat, section_beat);
  }
};

} // namespace frame
} // namespace dsp
} // namespace myrisa
