#ifndef __EXT_DATA_H__
#define __EXT_DATA_H__

/* External data section helper macro */
/* internal concat & strify */
#if (EXT_MEM_SIZE != 0 && EXT_MEM_BASE != 0)
#define __EXT_SECTION_TOKEN(x) .ext.mem._##x
#define __EXT_SECTION_STRIFY(x) #x
#define _EXT_SECTION_STRIFY(x) __EXT_SECTION_STRIFY(x)
#define _EXT_SECTION_NAME(x) _EXT_SECTION_STRIFY(__EXT_SECTION_TOKEN(x))
/* Use below macro to define a data into specific ext data region */
/* Usage:

    type_t data_A[array_len] EXT_SECTION(SPECIFIC_SECTION);

    * data_A will be insert into ext.data._SEPCIFIC_SECTION section.
    * Note: Only static data definition could be applied
    * Any initialized value will be removed.
 */
#define EXT_SECTION(x) __attribute__((section(_EXT_SECTION_NAME(x))))
#else
#define EXT_SECTION(x)
/* External data section helper macro end */
#endif

#endif
