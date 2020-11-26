#pragma once

// enum RecordMode { REPLACE, CREATE };
enum RecordMode { READ, DUB, EXTEND };
enum RecordContext { STRUCTURE, TIME };

struct Delta {
  RecordMode mode;
  RecordContext context;
  float attenuation;
  bool recording;
};

