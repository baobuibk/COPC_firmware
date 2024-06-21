/*
 * cmdline.h
 *
 *  Created on: May 23, 2024
 *      Author: CAO HIEU
 */
#include "main.h"

#ifndef CMDLINE_CMDLINE_H_
#define CMDLINE_CMDLINE_H_
#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! \addtogroup cmdline_api
//! @{
//
//*****************************************************************************
#define CMDLINE_OK        (0)
//*****************************************************************************
//
//! Defines the value that is returned if the command is not found.
//
//*****************************************************************************
#define CMDLINE_BAD_CMD         (1)

//*****************************************************************************
//
//! Defines the value that is returned if there are too many arguments.
//
//*****************************************************************************
#define CMDLINE_TOO_MANY_ARGS   (2)

//*****************************************************************************
//
//! Defines the value that is returned if there are too few arguments.
//
//*****************************************************************************
#define CMDLINE_TOO_FEW_ARGS   (3)

//*****************************************************************************
//
//! Defines the value that is returned if an argument is invalid.
//
//*****************************************************************************
#define CMDLINE_INVALID_ARG   (4)

#define CMDLINE_PENDING   (5)

//*****************************************************************************
//
// Command line function callback type.
//
//*****************************************************************************
typedef int (*pfnCmdLine)(int argc, char *argv[]);

//*****************************************************************************
//
//! Structure for an entry in the command list table.
//
//*****************************************************************************
typedef struct
{
    //
    //! A pointer to a string containing the name of the command.
    //
    const char *pcCmd;

    //
    //! A function pointer to the implementation of the command.
    //
    pfnCmdLine pfnCmd;

    //
    //! A pointer to a string of brief help text for the command.
    //
    const char *pcHelp;
}
tCmdLineEntry;

//*****************************************************************************
//
//! This is the command table that must be provided by the application.  The
//! last element of the array must be a structure whose pcCmd field contains
//! a NULL pointer.
//
//*****************************************************************************
extern tCmdLineEntry g_psCmdTable[];

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************
extern uint8_t CmdLineProcess(char *pcCmdLine, USART_TypeDef* USARTx);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif


#endif /* CMDLINE_CMDLINE_H_ */
