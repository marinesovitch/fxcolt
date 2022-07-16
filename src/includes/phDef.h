#ifndef INC_INCLUDES_PHDEF_H
#define INC_INCLUDES_PHDEF_H

# if defined _MSC_VER
#    pragma warning (disable: 4100 4127 4200 4201 4251 4275 4355 4511 4512 4584 4701 4702 4714 4815 4996) 
#    if defined NDEBUG
#        pragma inline_depth( 255 )
#    endif
#    if _MSC_VER >= 1400
#        define _SECURE_SCL 0
#    endif
# define _WIN32_DCOM // WMI issues
# endif

#endif
