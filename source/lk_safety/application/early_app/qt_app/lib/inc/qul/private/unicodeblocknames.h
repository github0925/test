/******************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Quick Ultralite module.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
******************************************************************************/

// Unicode block names from Blocks.txt
UnicodeBlock_BasicLatin, UnicodeBlock_Latin1Supplement, UnicodeBlock_LatinExtendedA, UnicodeBlock_LatinExtendedB,
    UnicodeBlock_IPAExtensions, UnicodeBlock_SpacingModifierLetters, UnicodeBlock_CombiningDiacriticalMarks,
    UnicodeBlock_GreekandCoptic, UnicodeBlock_Cyrillic, UnicodeBlock_CyrillicSupplement, UnicodeBlock_Armenian,
    UnicodeBlock_Hebrew, UnicodeBlock_Arabic, UnicodeBlock_Syriac, UnicodeBlock_ArabicSupplement, UnicodeBlock_Thaana,
    UnicodeBlock_NKo, UnicodeBlock_Samaritan, UnicodeBlock_Mandaic, UnicodeBlock_SyriacSupplement,
    UnicodeBlock_ArabicExtendedA, UnicodeBlock_Devanagari, UnicodeBlock_Bengali, UnicodeBlock_Gurmukhi,
    UnicodeBlock_Gujarati, UnicodeBlock_Oriya, UnicodeBlock_Tamil, UnicodeBlock_Telugu, UnicodeBlock_Kannada,
    UnicodeBlock_Malayalam, UnicodeBlock_Sinhala, UnicodeBlock_Thai, UnicodeBlock_Lao, UnicodeBlock_Tibetan,
    UnicodeBlock_Myanmar, UnicodeBlock_Georgian, UnicodeBlock_HangulJamo, UnicodeBlock_Ethiopic,
    UnicodeBlock_EthiopicSupplement, UnicodeBlock_Cherokee, UnicodeBlock_UnifiedCanadianAboriginalSyllabics,
    UnicodeBlock_Ogham, UnicodeBlock_Runic, UnicodeBlock_Tagalog, UnicodeBlock_Hanunoo, UnicodeBlock_Buhid,
    UnicodeBlock_Tagbanwa, UnicodeBlock_Khmer, UnicodeBlock_Mongolian,
    UnicodeBlock_UnifiedCanadianAboriginalSyllabicsExtended, UnicodeBlock_Limbu, UnicodeBlock_TaiLe,
    UnicodeBlock_NewTaiLue, UnicodeBlock_KhmerSymbols, UnicodeBlock_Buginese, UnicodeBlock_TaiTham,
    UnicodeBlock_CombiningDiacriticalMarksExtended, UnicodeBlock_Balinese, UnicodeBlock_Sundanese, UnicodeBlock_Batak,
    UnicodeBlock_Lepcha, UnicodeBlock_OlChiki, UnicodeBlock_CyrillicExtendedC, UnicodeBlock_GeorgianExtended,
    UnicodeBlock_SundaneseSupplement, UnicodeBlock_VedicExtensions, UnicodeBlock_PhoneticExtensions,
    UnicodeBlock_PhoneticExtensionsSupplement, UnicodeBlock_CombiningDiacriticalMarksSupplement,
    UnicodeBlock_LatinExtendedAdditional, UnicodeBlock_GreekExtended, UnicodeBlock_GeneralPunctuation,
    UnicodeBlock_SuperscriptsandSubscripts, UnicodeBlock_CurrencySymbols,
    UnicodeBlock_CombiningDiacriticalMarksforSymbols, UnicodeBlock_LetterlikeSymbols, UnicodeBlock_NumberForms,
    UnicodeBlock_Arrows, UnicodeBlock_MathematicalOperators, UnicodeBlock_MiscellaneousTechnical,
    UnicodeBlock_ControlPictures, UnicodeBlock_OpticalCharacterRecognition, UnicodeBlock_EnclosedAlphanumerics,
    UnicodeBlock_BoxDrawing, UnicodeBlock_BlockElements, UnicodeBlock_GeometricShapes,
    UnicodeBlock_MiscellaneousSymbols, UnicodeBlock_Dingbats, UnicodeBlock_MiscellaneousMathematicalSymbolsA,
    UnicodeBlock_SupplementalArrowsA, UnicodeBlock_BraillePatterns, UnicodeBlock_SupplementalArrowsB,
    UnicodeBlock_MiscellaneousMathematicalSymbolsB, UnicodeBlock_SupplementalMathematicalOperators,
    UnicodeBlock_MiscellaneousSymbolsandArrows, UnicodeBlock_Glagolitic, UnicodeBlock_LatinExtendedC,
    UnicodeBlock_Coptic, UnicodeBlock_GeorgianSupplement, UnicodeBlock_Tifinagh, UnicodeBlock_EthiopicExtended,
    UnicodeBlock_CyrillicExtendedA, UnicodeBlock_SupplementalPunctuation, UnicodeBlock_CJKRadicalsSupplement,
    UnicodeBlock_KangxiRadicals, UnicodeBlock_IdeographicDescriptionCharacters, UnicodeBlock_CJKSymbolsandPunctuation,
    UnicodeBlock_Hiragana, UnicodeBlock_Katakana, UnicodeBlock_Bopomofo, UnicodeBlock_HangulCompatibilityJamo,
    UnicodeBlock_Kanbun, UnicodeBlock_BopomofoExtended, UnicodeBlock_CJKStrokes,
    UnicodeBlock_KatakanaPhoneticExtensions, UnicodeBlock_EnclosedCJKLettersandMonths, UnicodeBlock_CJKCompatibility,
    UnicodeBlock_CJKUnifiedIdeographsExtensionA, UnicodeBlock_YijingHexagramSymbols, UnicodeBlock_CJKUnifiedIdeographs,
    UnicodeBlock_YiSyllables, UnicodeBlock_YiRadicals, UnicodeBlock_Lisu, UnicodeBlock_Vai,
    UnicodeBlock_CyrillicExtendedB, UnicodeBlock_Bamum, UnicodeBlock_ModifierToneLetters, UnicodeBlock_LatinExtendedD,
    UnicodeBlock_SylotiNagri, UnicodeBlock_CommonIndicNumberForms, UnicodeBlock_Phagspa, UnicodeBlock_Saurashtra,
    UnicodeBlock_DevanagariExtended, UnicodeBlock_KayahLi, UnicodeBlock_Rejang, UnicodeBlock_HangulJamoExtendedA,
    UnicodeBlock_Javanese, UnicodeBlock_MyanmarExtendedB, UnicodeBlock_Cham, UnicodeBlock_MyanmarExtendedA,
    UnicodeBlock_TaiViet, UnicodeBlock_MeeteiMayekExtensions, UnicodeBlock_EthiopicExtendedA,
    UnicodeBlock_LatinExtendedE, UnicodeBlock_CherokeeSupplement, UnicodeBlock_MeeteiMayek,
    UnicodeBlock_HangulSyllables, UnicodeBlock_HangulJamoExtendedB, UnicodeBlock_HighSurrogates,
    UnicodeBlock_HighPrivateUseSurrogates, UnicodeBlock_LowSurrogates, UnicodeBlock_PrivateUseArea,
    UnicodeBlock_CJKCompatibilityIdeographs, UnicodeBlock_AlphabeticPresentationForms,
    UnicodeBlock_ArabicPresentationFormsA, UnicodeBlock_VariationSelectors, UnicodeBlock_VerticalForms,
    UnicodeBlock_CombiningHalfMarks, UnicodeBlock_CJKCompatibilityForms, UnicodeBlock_SmallFormVariants,
    UnicodeBlock_ArabicPresentationFormsB, UnicodeBlock_HalfwidthandFullwidthForms, UnicodeBlock_Specials,
    UnicodeBlock_LinearBSyllabary, UnicodeBlock_LinearBIdeograms, UnicodeBlock_AegeanNumbers,
    UnicodeBlock_AncientGreekNumbers, UnicodeBlock_AncientSymbols, UnicodeBlock_PhaistosDisc, UnicodeBlock_Lycian,
    UnicodeBlock_Carian, UnicodeBlock_CopticEpactNumbers, UnicodeBlock_OldItalic, UnicodeBlock_Gothic,
    UnicodeBlock_OldPermic, UnicodeBlock_Ugaritic, UnicodeBlock_OldPersian, UnicodeBlock_Deseret, UnicodeBlock_Shavian,
    UnicodeBlock_Osmanya, UnicodeBlock_Osage, UnicodeBlock_Elbasan, UnicodeBlock_CaucasianAlbanian,
    UnicodeBlock_LinearA, UnicodeBlock_CypriotSyllabary, UnicodeBlock_ImperialAramaic, UnicodeBlock_Palmyrene,
    UnicodeBlock_Nabataean, UnicodeBlock_Hatran, UnicodeBlock_Phoenician, UnicodeBlock_Lydian,
    UnicodeBlock_MeroiticHieroglyphs, UnicodeBlock_MeroiticCursive, UnicodeBlock_Kharoshthi,
    UnicodeBlock_OldSouthArabian, UnicodeBlock_OldNorthArabian, UnicodeBlock_Manichaean, UnicodeBlock_Avestan,
    UnicodeBlock_InscriptionalParthian, UnicodeBlock_InscriptionalPahlavi, UnicodeBlock_PsalterPahlavi,
    UnicodeBlock_OldTurkic, UnicodeBlock_OldHungarian, UnicodeBlock_HanifiRohingya, UnicodeBlock_RumiNumeralSymbols,
    UnicodeBlock_OldSogdian, UnicodeBlock_Sogdian, UnicodeBlock_Elymaic, UnicodeBlock_Brahmi, UnicodeBlock_Kaithi,
    UnicodeBlock_SoraSompeng, UnicodeBlock_Chakma, UnicodeBlock_Mahajani, UnicodeBlock_Sharada,
    UnicodeBlock_SinhalaArchaicNumbers, UnicodeBlock_Khojki, UnicodeBlock_Multani, UnicodeBlock_Khudawadi,
    UnicodeBlock_Grantha, UnicodeBlock_Newa, UnicodeBlock_Tirhuta, UnicodeBlock_Siddham, UnicodeBlock_Modi,
    UnicodeBlock_MongolianSupplement, UnicodeBlock_Takri, UnicodeBlock_Ahom, UnicodeBlock_Dogra,
    UnicodeBlock_WarangCiti, UnicodeBlock_Nandinagari, UnicodeBlock_ZanabazarSquare, UnicodeBlock_Soyombo,
    UnicodeBlock_PauCinHau, UnicodeBlock_Bhaiksuki, UnicodeBlock_Marchen, UnicodeBlock_MasaramGondi,
    UnicodeBlock_GunjalaGondi, UnicodeBlock_Makasar, UnicodeBlock_TamilSupplement, UnicodeBlock_Cuneiform,
    UnicodeBlock_CuneiformNumbersandPunctuation, UnicodeBlock_EarlyDynasticCuneiform, UnicodeBlock_EgyptianHieroglyphs,
    UnicodeBlock_EgyptianHieroglyphFormatControls, UnicodeBlock_AnatolianHieroglyphs, UnicodeBlock_BamumSupplement,
    UnicodeBlock_Mro, UnicodeBlock_BassaVah, UnicodeBlock_PahawhHmong, UnicodeBlock_Medefaidrin, UnicodeBlock_Miao,
    UnicodeBlock_IdeographicSymbolsandPunctuation, UnicodeBlock_Tangut, UnicodeBlock_TangutComponents,
    UnicodeBlock_KanaSupplement, UnicodeBlock_KanaExtendedA, UnicodeBlock_SmallKanaExtension, UnicodeBlock_Nushu,
    UnicodeBlock_Duployan, UnicodeBlock_ShorthandFormatControls, UnicodeBlock_ByzantineMusicalSymbols,
    UnicodeBlock_MusicalSymbols, UnicodeBlock_AncientGreekMusicalNotation, UnicodeBlock_MayanNumerals,
    UnicodeBlock_TaiXuanJingSymbols, UnicodeBlock_CountingRodNumerals, UnicodeBlock_MathematicalAlphanumericSymbols,
    UnicodeBlock_SuttonSignWriting, UnicodeBlock_GlagoliticSupplement, UnicodeBlock_NyiakengPuachueHmong,
    UnicodeBlock_Wancho, UnicodeBlock_MendeKikakui, UnicodeBlock_Adlam, UnicodeBlock_IndicSiyaqNumbers,
    UnicodeBlock_OttomanSiyaqNumbers, UnicodeBlock_ArabicMathematicalAlphabeticSymbols, UnicodeBlock_MahjongTiles,
    UnicodeBlock_DominoTiles, UnicodeBlock_PlayingCards, UnicodeBlock_EnclosedAlphanumericSupplement,
    UnicodeBlock_EnclosedIdeographicSupplement, UnicodeBlock_MiscellaneousSymbolsandPictographs, UnicodeBlock_Emoticons,
    UnicodeBlock_OrnamentalDingbats, UnicodeBlock_TransportandMapSymbols, UnicodeBlock_AlchemicalSymbols,
    UnicodeBlock_GeometricShapesExtended, UnicodeBlock_SupplementalArrowsC,
    UnicodeBlock_SupplementalSymbolsandPictographs, UnicodeBlock_ChessSymbols,
    UnicodeBlock_SymbolsandPictographsExtendedA, UnicodeBlock_CJKUnifiedIdeographsExtensionB,
    UnicodeBlock_CJKUnifiedIdeographsExtensionC, UnicodeBlock_CJKUnifiedIdeographsExtensionD,
    UnicodeBlock_CJKUnifiedIdeographsExtensionE, UnicodeBlock_CJKUnifiedIdeographsExtensionF,
    UnicodeBlock_CJKCompatibilityIdeographsSupplement, UnicodeBlock_Tags, UnicodeBlock_VariationSelectorsSupplement,
    UnicodeBlock_SupplementaryPrivateUseAreaA, UnicodeBlock_SupplementaryPrivateUseAreaB,
