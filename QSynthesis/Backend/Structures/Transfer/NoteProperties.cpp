#include "NoteProperties.h"
#include "QUtauBasic.h"

NoteProperties::NoteProperties() {
    clear();
}

void NoteProperties::clear() {
    length = intensity = modulation = preUttr = overlap = flags = velocity = startpoint =
        preUttrOto = overlapOto = tempo = NODEF_STRING;
}
