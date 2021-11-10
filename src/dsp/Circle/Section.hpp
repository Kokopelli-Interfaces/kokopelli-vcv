#pragma once

#include "Song.hpp"
#include "definitions.hpp"

namespace kokopellivcv {
namespace dsp {
namespace circle {

struct Section {
  // TODO start_song
  // TimePosition _start;
  unsigned int start_beat = 0;
  unsigned int end_beat = 1;
  // unsigned int n_beats = 0;

  // std::vector<Cycle*> cycles_in_section;

  char group = 'A';
  int group_section_n = 1;

  Section *group_start_section = nullptr;
  Section *prev = nullptr;
  Section *next = nullptr;

  Section() {
    return;
  }

  inline unsigned int getNBeats() {
    assert(start_beat < end_beat);
    return end_beat - start_beat;
  }

  inline static Section* createNextGroupSection(Section* section, unsigned int start_beat) {
    Section* group_section = createNextSection(section, start_beat);
    group_section->group++;
    group_section->group_section_n = 1;
    group_section->group_start_section = group_section;
    group_section->end_beat = group_section->start_beat + 1;
    return group_section;
  }

  inline static Section* createNextSection(Section* section, unsigned int start_beat) {
    assert(section);

    Section *new_section = new Section();
    *new_section = *section;

    new_section->group_section_n++;

    Section* future_section = section->next;
    while (future_section) {
      if (future_section->group == section->group)  {
        future_section->group_section_n++;
      }
      future_section = future_section->next;
    }

    unsigned int n_beats = section->end_beat - section->start_beat;
    new_section->start_beat = start_beat;
    new_section->end_beat = start_beat + n_beats;

    section->next = new_section;
    new_section->prev = section;

    return new_section;
  }

  inline static Section* createNewSectionAfterSectionWithSameLength(Section* section) {
    assert(section);

    Section *new_section = new Section();
    *new_section = *section;

    new_section->group_section_n++;

    Section* future_section = section->next;
    while (future_section) {
      if (future_section->group == section->group)  {
        future_section->group_section_n++;
      }
      future_section = future_section->next;
    }

    unsigned int n_beats = section->end_beat - section->start_beat;
    new_section->start_beat += n_beats;
    new_section->end_beat += n_beats;

    section->next = new_section;
    new_section->prev = section;

    return new_section;
  }

  inline static Section* findNextSectionWithSameGroup(Section* section) {
    assert(section);

    Section* future_section = section->next;
    while (future_section) {
      if (future_section->group == section->group)  {
        break;
      }
      future_section = future_section->next;
    }

    if (future_section == nullptr) {
      assert(section->group_start_section);
      return section->group_start_section;
    }

    return future_section;
  }

  // TODO should it be this way? or different lists for each group
  //                    B1->B2
  // like    A1->A2->A3      ^C1-----
  //        A^--------->B^---C-------
  inline static Section* findNextSection(Section* section) {
    if (section->next) {
      return section->next;
    } else {
      Section* past_section = section;
      while (past_section) {
        if (!past_section->prev) {
          return past_section;
        }
        past_section = past_section->prev;
      }

      assert(true == false);
      return nullptr;
    }
  }
};

} // namespace circle
} // namespace dsp
} // namespace kokopellivcv
