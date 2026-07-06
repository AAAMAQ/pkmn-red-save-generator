#pragma once

#include <array>
#include <cstddef>

namespace pkmn::savegen::encoding {

struct Gen1Layout {
    static constexpr std::size_t ExpectedSaveSize = 0x8000;

    static constexpr std::size_t HallOfFameOff = 0x0598;
    static constexpr std::size_t HallOfFameLen = 0x12C0;

    static constexpr std::size_t TrainerNameOff = 0x2598;
    static constexpr std::size_t TrainerNameLen = 11;
    static constexpr std::size_t PokedexOwnedOff = 0x25A3;
    static constexpr std::size_t PokedexSeenOff = 0x25B6;
    static constexpr std::size_t PokedexBitsLen = 0x13;
    static constexpr std::size_t BagItemsCountOff = 0x25C9;
    static constexpr std::size_t BagItemsPairsOff = 0x25CA;
    static constexpr int BagItemsMaxPairs = 20;
    static constexpr std::size_t MoneyOff = 0x25F3;
    static constexpr std::size_t RivalNameOff = 0x25F6;
    static constexpr std::size_t RivalNameLen = 11;
    static constexpr std::size_t OptionsOff = 0x2601;
    static constexpr std::size_t BadgesOff = 0x2602;
    static constexpr std::size_t BadgesMirrorOff = 0x29D6;
    static constexpr std::size_t LetterDelayOff = 0x2604;
    static constexpr std::size_t TrainerIdOff = 0x2605;
    static constexpr std::size_t ContrastOff = 0x2609;
    static constexpr std::size_t MapIdOff = 0x260A;
    static constexpr std::size_t YCoordOff = 0x260D;
    static constexpr std::size_t XCoordOff = 0x260E;
    static constexpr std::size_t YBlockCoordOff = 0x260F;
    static constexpr std::size_t XBlockCoordOff = 0x2610;
    static constexpr std::size_t PreviousMapOff = 0x2611;
    static constexpr std::size_t PCItemBoxCountOff = 0x27E6;
    static constexpr std::size_t PCItemBoxPairsOff = 0x27E7;
    static constexpr int PCItemBoxMaxPairs = 50;
    static constexpr std::size_t PCItemBoxSerializedLen = 1 + (PCItemBoxMaxPairs * 2) + 1;
    static constexpr std::size_t CurrentBoxByteOff = 0x284C;
    static constexpr std::size_t HallOfFameRecordCountOff = 0x284E;
    static constexpr std::size_t CoinsOff = 0x2850;
    static constexpr std::size_t HiddenItemsOff = 0x299C;
    static constexpr std::size_t HiddenItemsLen = 7;
    static constexpr int HiddenItemsUsedBits = 54;
    static constexpr std::size_t HiddenCoinsOff = 0x29AA;
    static constexpr std::size_t HiddenCoinsLen = 2;
    static constexpr int HiddenCoinsUsedBits = 12;
    static constexpr std::size_t VisitedTownsOff = 0x29B7;
    static constexpr std::size_t VisitedTownsLen = 2;
    static constexpr int VisitedTownsUsedBits = 11;
    static constexpr std::size_t PlayHoursOff = 0x2CED;
    static constexpr std::size_t PlayMaxedOff = 0x2CEE;
    static constexpr std::size_t PlayMinutesOff = 0x2CEF;
    static constexpr std::size_t PlaySecondsOff = 0x2CF0;
    static constexpr std::size_t PlayFramesOff = 0x2CF1;
    static constexpr std::size_t DaycareInUseOff = 0x2CF4;
    static constexpr std::size_t DaycareNicknameOff = 0x2CF5;
    static constexpr std::size_t DaycareOTNameOff = 0x2D00;
    static constexpr std::size_t DaycareBoxMonOff = 0x2D0B;
    static constexpr std::size_t DaycareLen = 0x38;
    static constexpr std::size_t PartyBase = 0x2F2C;
    static constexpr std::size_t PartyBlockLen = 0x0194;
    static constexpr std::size_t PartyCountOff = PartyBase + 0x00;
    static constexpr std::size_t PartySpeciesOff = PartyBase + 0x01;
    static constexpr std::size_t PartySpeciesTerminatorOff = PartyBase + 0x07;
    static constexpr std::size_t PartyStructsOff = PartyBase + 0x08;
    static constexpr std::size_t PartyStructSize = 0x002C;
    static constexpr std::size_t PartyOTNamesOff = PartyBase + 0x0110;
    static constexpr std::size_t PartyNicknamesOff = PartyBase + 0x0152;
    static constexpr std::size_t Gen1NameLen = 0x0B;
    static constexpr std::size_t PartyMonSpeciesRel = 0x00;
    static constexpr std::size_t PartyMonCurrentHpRel = 0x01;
    static constexpr std::size_t PartyMonBoxLevelRel = 0x03;
    static constexpr std::size_t PartyMonStatusRel = 0x04;
    static constexpr std::size_t PartyMonType1Rel = 0x05;
    static constexpr std::size_t PartyMonType2Rel = 0x06;
    static constexpr std::size_t PartyMonCatchRateRel = 0x07;
    static constexpr std::size_t PartyMonMovesRel = 0x08;
    static constexpr std::size_t PartyMonTrainerIdRel = 0x0C;
    static constexpr std::size_t PartyMonExperienceRel = 0x0E;
    static constexpr std::size_t PartyMonHpStatExpRel = 0x11;
    static constexpr std::size_t PartyMonAttackStatExpRel = 0x13;
    static constexpr std::size_t PartyMonDefenseStatExpRel = 0x15;
    static constexpr std::size_t PartyMonSpeedStatExpRel = 0x17;
    static constexpr std::size_t PartyMonSpecialStatExpRel = 0x19;
    static constexpr std::size_t PartyMonDvWordRel = 0x1B;
    static constexpr std::size_t PartyMonPpRel = 0x1D;
    static constexpr std::size_t PartyMonLevelRel = 0x21;
    static constexpr std::size_t PartyMonMaxHpRel = 0x22;
    static constexpr std::size_t PartyMonAttackRel = 0x24;
    static constexpr std::size_t PartyMonDefenseRel = 0x26;
    static constexpr std::size_t PartyMonSpeedRel = 0x28;
    static constexpr std::size_t PartyMonSpecialRel = 0x2A;
    static constexpr std::size_t CurrentBoxCacheOff = 0x30C0;
    static constexpr std::size_t CurrentBoxCacheLen = 0x0462;
    static constexpr int BoxMaxMons = 20;
    static constexpr std::size_t BoxBlockSize = 0x0462;
    static constexpr std::size_t BoxCountRel = 0x0000;
    static constexpr std::size_t MainChecksumStart = 0x2598;
    static constexpr std::size_t MainChecksumEndInclusive = 0x3522;
    static constexpr std::size_t MainChecksumOff = 0x3523;
    static constexpr std::size_t Bank2AllChecksumOff = 0x5A4C;
    static constexpr std::size_t Bank2PayloadStart = 0x4000;
    static constexpr std::size_t Bank2PayloadEndInclusive = 0x5A4B;
    static constexpr std::size_t Bank3AllChecksumOff = 0x7A4C;
    static constexpr std::size_t Bank3PayloadStart = 0x6000;
    static constexpr std::size_t Bank3PayloadEndInclusive = 0x7A4B;

    static constexpr std::array<std::size_t, 12> PermanentBoxOffsets = {
        0x4000, 0x4462, 0x48C4, 0x4D26, 0x5188, 0x55EA,
        0x6000, 0x6462, 0x68C4, 0x6D26, 0x7188, 0x75EA
    };
};

}  // namespace pkmn::savegen::encoding
