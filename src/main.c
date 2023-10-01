#include <stdio.h>
#include <string.h>
#include "ygo_card.h"

void hd(const void* data, size_t size) {
  unsigned char ascii[17];
  size_t i, j;
  ascii[16] = '\0';
  for (i = 0; i < size; ++i) {
    printf("%02X ", ((unsigned char*)data)[i]);
    if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
      ascii[i % 16] = ((unsigned char*)data)[i];
    } else {
      ascii[i % 16] = '.';
    }
    if ((i+1) % 8 == 0 || i+1 == size) {
      printf(" ");
      if ((i+1) % 16 == 0) {
        printf("|  %s \n", ascii);
      } else if (i+1 == size) {
        ascii[(i+1) % 16] = '\0';
        if ((i+1) % 16 <= 8) {
          printf(" ");
        }
        for (j = (i+1) % 16; j < 16; ++j) {
          printf("   ");
        }
        printf("|  %s \n", ascii);
      }
    }
  }
}

int main() {
  ygo_card_t card = {
      .id = 40737112,
      .name = "Dark Magician of Chaos",
      .type = YGO_CARD_TYPE_EFFECT_MONSTER,
      .frame = YGO_CARD_FRAME_EFFECT,
      .subtype = YGO_CARD_STYPE_MONSTER_SPELLCASTER,
      .attribute = YGO_CARD_ATTR_DARK,
      .atk = 2800,
      .def = 2600,
      .rank = 8,
      .scale = 0,
      .link_value = 0,
      .link_markers = 0
  };

  printf("Loaded card: %s\n", card.name);

  uint8_t raw[144] = {0};
  ygo_card_serialize(raw, &card);

  printf("Serializing as raw:\n");
  hd(raw, sizeof(raw));

  return 0;
}
