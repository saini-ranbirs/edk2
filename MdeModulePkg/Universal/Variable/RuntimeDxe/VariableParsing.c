/** @file
  Functions in this module are associated with variable parsing operations and
  are intended to be usable across variable driver source files.

Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "VariableParsing.h"
#include <Guid/SmmVariableCommon.h>

#define ENABLE_RS_SVA  1

#if ENABLE_RS_SVA
extern UINT8 mVarCount;
extern UINT8 mVarBuffer[45][700];
extern UINT8 mVarCurr;
#endif

static void memdump(const void *src, UINTN count, char *name);

static void memdump(const void *src, UINTN count, char *name)
{
#define BFR_DATA_LIMIT  16

	const char *temp = src;
	unsigned char bfr_data[BFR_DATA_LIMIT];
	UINTN bfr_counter, bfr_counter_limit;
	UINTN remaining = count, loop_count = 0;

	DEBUG((DEBUG_INFO,"\nEDK2 %s : %06lu %d", name, count, sizeof(SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE)));

	while (remaining) {
		bfr_counter = 0;

		(remaining >= BFR_DATA_LIMIT) ?
		    (bfr_counter_limit = BFR_DATA_LIMIT) :
		    (bfr_counter_limit = remaining);

		while (bfr_counter < bfr_counter_limit) {
			bfr_data[bfr_counter] = *(temp + bfr_counter);
			bfr_counter++;
		}

		/* For simplicity, fill rest with ZERO's if required */
		while (bfr_counter < BFR_DATA_LIMIT) {
			bfr_data[bfr_counter] = 0x00;
			bfr_counter++;
		}

		if (loop_count < 45) {
		DEBUG((DEBUG_INFO,"\n%p: %06lu "
			"%02x%02x %02x%02x %02x%02x %02x%02x "
			"%02x%02x %02x%02x %02x%02x %02x%02x",
			temp, count - remaining,
			bfr_data[1], bfr_data[0], bfr_data[3], bfr_data[2],
			bfr_data[5], bfr_data[4], bfr_data[7], bfr_data[6],
			bfr_data[9], bfr_data[8], bfr_data[11], bfr_data[10],
			bfr_data[13], bfr_data[12], bfr_data[15], bfr_data[14]));
		}

		temp = temp + bfr_counter_limit;
		remaining = remaining - bfr_counter_limit;
		loop_count++;
	}

	DEBUG((DEBUG_INFO,"\n%p: %06lu\n", temp, count - remaining));
}

/**

  This code checks if variable header is valid or not.

  @param[in] Variable           Pointer to the Variable Header.
  @param[in] VariableStoreEnd   Pointer to the Variable Store End.

  @retval TRUE              Variable header is valid.
  @retval FALSE             Variable header is not valid.

**/
BOOLEAN
IsValidVariableHeader (
  IN  VARIABLE_HEADER  *Variable,
  IN  VARIABLE_HEADER  *VariableStoreEnd
  )
{
  if ((Variable == NULL) || (Variable >= VariableStoreEnd) || (Variable->StartId != VARIABLE_DATA)) {
    //
    // Variable is NULL or has reached the end of variable store,
    // or the StartId is not correct.
    //
    return FALSE;
  }

  return TRUE;
}

/**

  This code gets the current status of Variable Store.

  @param[in] VarStoreHeader  Pointer to the Variable Store Header.

  @retval EfiRaw         Variable store status is raw.
  @retval EfiValid       Variable store status is valid.
  @retval EfiInvalid     Variable store status is invalid.

**/
VARIABLE_STORE_STATUS
GetVariableStoreStatus (
  IN VARIABLE_STORE_HEADER  *VarStoreHeader
  )
{
  if ((CompareGuid (&VarStoreHeader->Signature, &gEfiAuthenticatedVariableGuid) ||
       CompareGuid (&VarStoreHeader->Signature, &gEfiVariableGuid)) &&
      (VarStoreHeader->Format == VARIABLE_STORE_FORMATTED) &&
      (VarStoreHeader->State == VARIABLE_STORE_HEALTHY)
      )
  {
    return EfiValid;
  } else if ((((UINT32 *)(&VarStoreHeader->Signature))[0] == 0xffffffff) &&
             (((UINT32 *)(&VarStoreHeader->Signature))[1] == 0xffffffff) &&
             (((UINT32 *)(&VarStoreHeader->Signature))[2] == 0xffffffff) &&
             (((UINT32 *)(&VarStoreHeader->Signature))[3] == 0xffffffff) &&
             (VarStoreHeader->Size == 0xffffffff) &&
             (VarStoreHeader->Format == 0xff) &&
             (VarStoreHeader->State == 0xff)
             )
  {
    return EfiRaw;
  } else {
    return EfiInvalid;
  }
}

/**
  This code gets the size of variable header.

  @param[in]  AuthFormat    TRUE indicates authenticated variables are used.
                            FALSE indicates authenticated variables are not used.

  @return Size of variable header in bytes in type UINTN.

**/
UINTN
GetVariableHeaderSize (
  IN  BOOLEAN  AuthFormat
  )
{
  UINTN  Value;

  if (AuthFormat) {
    Value = sizeof (AUTHENTICATED_VARIABLE_HEADER);
  } else {
    Value = sizeof (VARIABLE_HEADER);
  }

  return Value;
}

/**

  This code gets the size of name of variable.

  @param[in]  Variable      Pointer to the variable header.
  @param[in]  AuthFormat    TRUE indicates authenticated variables are used.
                            FALSE indicates authenticated variables are not used.

  @return UINTN          Size of variable in bytes.

**/
UINTN
NameSizeOfVariable (
  IN  VARIABLE_HEADER  *Variable,
  IN  BOOLEAN          AuthFormat
  )
{
  AUTHENTICATED_VARIABLE_HEADER  *AuthVariable;

  AuthVariable = (AUTHENTICATED_VARIABLE_HEADER *)Variable;
  if (AuthFormat) {
    if ((AuthVariable->State == (UINT8)(-1)) ||
        (AuthVariable->DataSize == (UINT32)(-1)) ||
        (AuthVariable->NameSize == (UINT32)(-1)) ||
        (AuthVariable->Attributes == (UINT32)(-1)))
    {
      return 0;
    }

    return (UINTN)AuthVariable->NameSize;
  } else {
    if ((Variable->State == (UINT8)(-1)) ||
        (Variable->DataSize == (UINT32)(-1)) ||
        (Variable->NameSize == (UINT32)(-1)) ||
        (Variable->Attributes == (UINT32)(-1)))
    {
      return 0;
    }

    return (UINTN)Variable->NameSize;
  }
}

/**
  This code sets the size of name of variable.

  @param[in]  Variable      Pointer to the Variable Header.
  @param[in]  NameSize      Name size to set.
  @param[in]  AuthFormat    TRUE indicates authenticated variables are used.
                            FALSE indicates authenticated variables are not used.

**/
VOID
SetNameSizeOfVariable (
  IN VARIABLE_HEADER  *Variable,
  IN UINTN            NameSize,
  IN BOOLEAN          AuthFormat
  )
{
  AUTHENTICATED_VARIABLE_HEADER  *AuthVariable;

  AuthVariable = (AUTHENTICATED_VARIABLE_HEADER *)Variable;
  if (AuthFormat) {
    AuthVariable->NameSize = (UINT32)NameSize;
  } else {
    Variable->NameSize = (UINT32)NameSize;
  }
}

/**

  This code gets the size of variable data.

  @param[in]  Variable      Pointer to the Variable Header.
  @param[in]  AuthFormat    TRUE indicates authenticated variables are used.
                            FALSE indicates authenticated variables are not used.

  @return Size of variable in bytes.

**/
UINTN
DataSizeOfVariable (
  IN  VARIABLE_HEADER  *Variable,
  IN  BOOLEAN          AuthFormat
  )
{
  AUTHENTICATED_VARIABLE_HEADER  *AuthVariable;

  AuthVariable = (AUTHENTICATED_VARIABLE_HEADER *)Variable;
  if (AuthFormat) {
    if ((AuthVariable->State == (UINT8)(-1)) ||
        (AuthVariable->DataSize == (UINT32)(-1)) ||
        (AuthVariable->NameSize == (UINT32)(-1)) ||
        (AuthVariable->Attributes == (UINT32)(-1)))
    {
      return 0;
    }

    return (UINTN)AuthVariable->DataSize;
  } else {
    if ((Variable->State == (UINT8)(-1)) ||
        (Variable->DataSize == (UINT32)(-1)) ||
        (Variable->NameSize == (UINT32)(-1)) ||
        (Variable->Attributes == (UINT32)(-1)))
    {
      return 0;
    }

    return (UINTN)Variable->DataSize;
  }
}

/**
  This code sets the size of variable data.

  @param[in] Variable   Pointer to the Variable Header.
  @param[in] DataSize   Data size to set.
  @param[in] AuthFormat TRUE indicates authenticated variables are used.
                        FALSE indicates authenticated variables are not used.

**/
VOID
SetDataSizeOfVariable (
  IN  VARIABLE_HEADER  *Variable,
  IN  UINTN            DataSize,
  IN  BOOLEAN          AuthFormat
  )
{
  AUTHENTICATED_VARIABLE_HEADER  *AuthVariable;

  AuthVariable = (AUTHENTICATED_VARIABLE_HEADER *)Variable;
  if (AuthFormat) {
    AuthVariable->DataSize = (UINT32)DataSize;
  } else {
    Variable->DataSize = (UINT32)DataSize;
  }
}

/**

  This code gets the pointer to the variable name.

  @param[in] Variable     Pointer to the Variable Header.
  @param[in] AuthFormat   TRUE indicates authenticated variables are used.
                          FALSE indicates authenticated variables are not used.

  @return Pointer to Variable Name which is Unicode encoding.

**/
CHAR16 *
GetVariableNamePtr (
  IN  VARIABLE_HEADER  *Variable,
  IN  BOOLEAN          AuthFormat
  )
{
  return (CHAR16 *)((UINTN)Variable + GetVariableHeaderSize (AuthFormat));
}

/**
  This code gets the pointer to the variable guid.

  @param[in] Variable     Pointer to the Variable Header.
  @param[in] AuthFormat   TRUE indicates authenticated variables are used.
                          FALSE indicates authenticated variables are not used.

  @return A EFI_GUID* pointer to Vendor Guid.

**/
EFI_GUID *
GetVendorGuidPtr (
  IN  VARIABLE_HEADER  *Variable,
  IN  BOOLEAN          AuthFormat
  )
{
  AUTHENTICATED_VARIABLE_HEADER  *AuthVariable;

  AuthVariable = (AUTHENTICATED_VARIABLE_HEADER *)Variable;
  if (AuthFormat) {
    return &AuthVariable->VendorGuid;
  } else {
    return &Variable->VendorGuid;
  }
}

/**

  This code gets the pointer to the variable data.

  @param[in] Variable     Pointer to the Variable Header.
  @param[in] AuthFormat   TRUE indicates authenticated variables are used.
                          FALSE indicates authenticated variables are not used.

  @return Pointer to Variable Data.

**/
UINT8 *
GetVariableDataPtr (
  IN  VARIABLE_HEADER  *Variable,
  IN  BOOLEAN          AuthFormat
  )
{
  UINTN  Value;

  //
  // Be careful about pad size for alignment.
  //
  Value  =  (UINTN)GetVariableNamePtr (Variable, AuthFormat);
  Value += NameSizeOfVariable (Variable, AuthFormat);
  Value += GET_PAD_SIZE (NameSizeOfVariable (Variable, AuthFormat));

  return (UINT8 *)Value;
}

/**
  This code gets the variable data offset related to variable header.

  @param[in] Variable     Pointer to the Variable Header.
  @param[in] AuthFormat   TRUE indicates authenticated variables are used.
                          FALSE indicates authenticated variables are not used.

  @return Variable Data offset.

**/
UINTN
GetVariableDataOffset (
  IN  VARIABLE_HEADER  *Variable,
  IN  BOOLEAN          AuthFormat
  )
{
  UINTN  Value;

  //
  // Be careful about pad size for alignment
  //
  Value  = GetVariableHeaderSize (AuthFormat);
  Value += NameSizeOfVariable (Variable, AuthFormat);
  Value += GET_PAD_SIZE (NameSizeOfVariable (Variable, AuthFormat));

  return Value;
}

/**

  This code gets the pointer to the next variable header.

  @param[in] Variable     Pointer to the Variable Header.
  @param[in] AuthFormat   TRUE indicates authenticated variables are used.
                          FALSE indicates authenticated variables are not used.

  @return Pointer to next variable header.

**/
VARIABLE_HEADER *
GetNextVariablePtr (
  IN  VARIABLE_HEADER  *Variable,
  IN  BOOLEAN          AuthFormat
  )
{
  UINTN  Value;

  Value  =  (UINTN)GetVariableDataPtr (Variable, AuthFormat);
  Value += DataSizeOfVariable (Variable, AuthFormat);
  Value += GET_PAD_SIZE (DataSizeOfVariable (Variable, AuthFormat));

  //
  // Be careful about pad size for alignment.
  //
  return (VARIABLE_HEADER *)HEADER_ALIGN (Value);
}

/**

  Gets the pointer to the first variable header in given variable store area.

  @param[in] VarStoreHeader  Pointer to the Variable Store Header.

  @return Pointer to the first variable header.

**/
VARIABLE_HEADER *
GetStartPointer (
  IN VARIABLE_STORE_HEADER  *VarStoreHeader
  )
{
  //
  // The start of variable store.
  //
  return (VARIABLE_HEADER *)HEADER_ALIGN (VarStoreHeader + 1);
}

/**

  Gets the pointer to the end of the variable storage area.

  This function gets pointer to the end of the variable storage
  area, according to the input variable store header.

  @param[in] VarStoreHeader  Pointer to the Variable Store Header.

  @return Pointer to the end of the variable storage area.

**/
VARIABLE_HEADER *
GetEndPointer (
  IN VARIABLE_STORE_HEADER  *VarStoreHeader
  )
{
  //
  // The end of variable store
  //
  return (VARIABLE_HEADER *)HEADER_ALIGN ((UINTN)VarStoreHeader + VarStoreHeader->Size);
}

/**
  Compare two EFI_TIME data.


  @param[in] FirstTime       A pointer to the first EFI_TIME data.
  @param[in] SecondTime      A pointer to the second EFI_TIME data.

  @retval  TRUE              The FirstTime is not later than the SecondTime.
  @retval  FALSE             The FirstTime is later than the SecondTime.

**/
BOOLEAN
VariableCompareTimeStampInternal (
  IN EFI_TIME  *FirstTime,
  IN EFI_TIME  *SecondTime
  )
{
  if (FirstTime->Year != SecondTime->Year) {
    return (BOOLEAN)(FirstTime->Year < SecondTime->Year);
  } else if (FirstTime->Month != SecondTime->Month) {
    return (BOOLEAN)(FirstTime->Month < SecondTime->Month);
  } else if (FirstTime->Day != SecondTime->Day) {
    return (BOOLEAN)(FirstTime->Day < SecondTime->Day);
  } else if (FirstTime->Hour != SecondTime->Hour) {
    return (BOOLEAN)(FirstTime->Hour < SecondTime->Hour);
  } else if (FirstTime->Minute != SecondTime->Minute) {
    return (BOOLEAN)(FirstTime->Minute < SecondTime->Minute);
  }

  return (BOOLEAN)(FirstTime->Second <= SecondTime->Second);
}

/**
  Find the variable in the specified variable store.

  @param[in]       VariableName        Name of the variable to be found
  @param[in]       VendorGuid          Vendor GUID to be found.
  @param[in]       IgnoreRtCheck       Ignore EFI_VARIABLE_RUNTIME_ACCESS attribute
                                       check at runtime when searching variable.
  @param[in, out]  PtrTrack            Variable Track Pointer structure that contains Variable Information.
  @param[in]       AuthFormat          TRUE indicates authenticated variables are used.
                                       FALSE indicates authenticated variables are not used.

  @retval          EFI_SUCCESS         Variable found successfully
  @retval          EFI_NOT_FOUND       Variable not found
**/
EFI_STATUS
FindVariableEx (
  IN     CHAR16                  *VariableName,
  IN     EFI_GUID                *VendorGuid,
  IN     BOOLEAN                 IgnoreRtCheck,
  IN OUT VARIABLE_POINTER_TRACK  *PtrTrack,
  IN     BOOLEAN                 AuthFormat
  )
{
  VARIABLE_HEADER  *InDeletedVariable;
  VOID             *Point;
  VARIABLE_HEADER  *Next2NextVarHdrPtr;

  PtrTrack->InDeletedTransitionPtr = NULL;

  //
  // Find the variable by walk through HOB, volatile and non-volatile variable store.
  //
  InDeletedVariable = NULL;

  for ( PtrTrack->CurrPtr = PtrTrack->StartPtr
        ; IsValidVariableHeader (PtrTrack->CurrPtr, PtrTrack->EndPtr)
        ; PtrTrack->CurrPtr = GetNextVariablePtr (PtrTrack->CurrPtr, AuthFormat)
        )
  {
    DEBUG ((DEBUG_INFO, "Ranbir: FindVariableEx %d initiating memdump %d %d\n", __LINE__, PtrTrack->CurrPtr->NameSize, PtrTrack->CurrPtr->DataSize));
    memdump(PtrTrack->CurrPtr, sizeof(VARIABLE_HEADER) + PtrTrack->CurrPtr->NameSize + PtrTrack->CurrPtr->DataSize, "H1");
    Next2NextVarHdrPtr = GetNextVariablePtr (PtrTrack->CurrPtr, AuthFormat);
    if (IsValidVariableHeader (Next2NextVarHdrPtr, PtrTrack->EndPtr)) {
      memdump(Next2NextVarHdrPtr, sizeof(VARIABLE_HEADER) + Next2NextVarHdrPtr->NameSize + Next2NextVarHdrPtr->DataSize, "H2");
    }
    if ((PtrTrack->CurrPtr->State == VAR_ADDED) ||
        (PtrTrack->CurrPtr->State == (VAR_IN_DELETED_TRANSITION & VAR_ADDED))
        )
    {
      if (IgnoreRtCheck || !AtRuntime () || ((PtrTrack->CurrPtr->Attributes & EFI_VARIABLE_RUNTIME_ACCESS) != 0)) {
        if (VariableName[0] == 0) {
          if (PtrTrack->CurrPtr->State == (VAR_IN_DELETED_TRANSITION & VAR_ADDED)) {
            InDeletedVariable = PtrTrack->CurrPtr;
            DEBUG ((DEBUG_INFO, "Ranbir: FindVariableEx %d\n", __LINE__));
          } else {
            PtrTrack->InDeletedTransitionPtr = InDeletedVariable;
            DEBUG ((DEBUG_INFO, "Ranbir: FindVariableEx %d PtrTrack->CurrPtr = 0x%x\n", __LINE__, PtrTrack->CurrPtr));
            return EFI_SUCCESS;
          }
        } else {
          if (CompareGuid (VendorGuid, GetVendorGuidPtr (PtrTrack->CurrPtr, AuthFormat))) {
            Point = (VOID *)GetVariableNamePtr (PtrTrack->CurrPtr, AuthFormat);

            ASSERT (NameSizeOfVariable (PtrTrack->CurrPtr, AuthFormat) != 0);
            if (CompareMem (VariableName, Point, NameSizeOfVariable (PtrTrack->CurrPtr, AuthFormat)) == 0) {
              if (PtrTrack->CurrPtr->State == (VAR_IN_DELETED_TRANSITION & VAR_ADDED)) {
                InDeletedVariable = PtrTrack->CurrPtr;
                DEBUG ((DEBUG_INFO, "Ranbir: FindVariableEx %d PtrTrack->CurrPtr = 0x%x\n", __LINE__, PtrTrack->CurrPtr));
              } else {
                PtrTrack->InDeletedTransitionPtr = InDeletedVariable;
                DEBUG ((DEBUG_INFO, "Ranbir: FindVariableEx %d PtrTrack->CurrPtr = 0x%x\n", __LINE__, PtrTrack->CurrPtr));
                return EFI_SUCCESS;
              }
            } else {
              DEBUG ((DEBUG_INFO, "Ranbir: FindVariableEx Variable Name Mismatch %d\n", __LINE__));
            }
          } else {
            DEBUG ((DEBUG_INFO, "Ranbir: FindVariableEx Vendor GUID Mismatch %d\n", __LINE__));
            memdump(GetVendorGuidPtr (PtrTrack->CurrPtr, AuthFormat), 16, "1");
          }
        }
      }
    }
  }

  PtrTrack->CurrPtr = InDeletedVariable;
  DEBUG ((DEBUG_INFO, "Ranbir: FindVariableEx %d Status = %r PtrTrack->CurrPtr = 0x%x\n", __LINE__, (PtrTrack->CurrPtr  == NULL) ? EFI_NOT_FOUND : EFI_SUCCESS, PtrTrack->CurrPtr));
  return (PtrTrack->CurrPtr  == NULL) ? EFI_NOT_FOUND : EFI_SUCCESS;
}

#if ENABLE_RS_SVA
EFI_STATUS
FindVariableRS (
  IN CHAR16    *VariableName,
  IN EFI_GUID  *VendorGuid
  )
{
  SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE *pSVCAVar;

  while (mVarCurr < mVarCount) {
    if (VariableName[0] == 0) {
      DEBUG ((DEBUG_INFO, "Ranbir: FindVariableRS L%d = EFI_SUCCESS mVarCurr = %d\n", __LINE__, mVarCurr));
      return EFI_SUCCESS;
    } else {
      pSVCAVar = (SMM_VARIABLE_COMMUNICATE_ACCESS_VARIABLE *)&mVarBuffer[mVarCurr];
      memdump(pSVCAVar, 16, "1");
      if (CompareGuid (VendorGuid, (const GUID *)pSVCAVar)) {
        ASSERT ((pSVCAVar->NameSize) != 0);
        memdump(pSVCAVar->Name, pSVCAVar->NameSize, "1");
        if (CompareMem (VariableName, pSVCAVar->Name, pSVCAVar->NameSize) == 0) {
          DEBUG ((DEBUG_INFO, "Ranbir: FindVariableRS L%d = EFI_SUCCESS mVarCurr = %d\n", __LINE__, mVarCurr));
          return EFI_SUCCESS;
        } else {
          DEBUG ((DEBUG_INFO, "Ranbir: FindVariableRS Variable Name Mismatch L%d\n", __LINE__));
        }
      } else {
        DEBUG ((DEBUG_INFO, "Ranbir: FindVariableRS Vendor GUID Mismatch L%d\n", __LINE__));
      }
    }
    mVarCurr++;
  }

  if (mVarCurr >= mVarCount)
    DEBUG ((DEBUG_INFO, "Ranbir: FindVariableRS L%d = EFI_NOT_FOUND mVarCurr = %d\n", __LINE__, mVarCurr));
  else
    DEBUG ((DEBUG_INFO, "Ranbir: FindVariableRS L%d = EFI_SUCCESS mVarCurr = %d\n", __LINE__, mVarCurr));
  return (mVarCurr >= mVarCount) ? EFI_NOT_FOUND : EFI_SUCCESS;
}
#endif

/**
  This code finds the next available variable.

  Caution: This function may receive untrusted input.
  This function may be invoked in SMM mode. This function will do basic validation, before parse the data.

  @param[in]  VariableName      Pointer to variable name.
  @param[in]  VendorGuid        Variable Vendor Guid.
  @param[in]  VariableStoreList A list of variable stores that should be used to get the next variable.
                                The maximum number of entries is the max value of VARIABLE_STORE_TYPE.
  @param[out] VariablePtr       Pointer to variable header address.
  @param[in]  AuthFormat        TRUE indicates authenticated variables are used.
                                FALSE indicates authenticated variables are not used.

  @retval EFI_SUCCESS           The function completed successfully.
  @retval EFI_NOT_FOUND         The next variable was not found.
  @retval EFI_INVALID_PARAMETER If VariableName is not an empty string, while VendorGuid is NULL.
  @retval EFI_INVALID_PARAMETER The input values of VariableName and VendorGuid are not a name and
                                GUID of an existing variable.

**/
EFI_STATUS
EFIAPI
VariableServiceGetNextVariableInternal (
  IN  CHAR16                 *VariableName,
  IN  EFI_GUID               *VendorGuid,
  IN  VARIABLE_STORE_HEADER  **VariableStoreList,
  OUT VARIABLE_HEADER        **VariablePtr,
  IN  BOOLEAN                AuthFormat
  )
#if !ENABLE_RS_SVA
{
  EFI_STATUS              Status;
  VARIABLE_STORE_TYPE     StoreType;
  VARIABLE_POINTER_TRACK  Variable;
  VARIABLE_POINTER_TRACK  VariableInHob;
  VARIABLE_POINTER_TRACK  VariablePtrTrack;
  UINTN                   LoopCount = 0;

  Status = EFI_NOT_FOUND;

  if (VariableStoreList == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ZeroMem (&Variable, sizeof (Variable));

  // Check if the variable exists in the given variable store list
  for (StoreType = (VARIABLE_STORE_TYPE)0; StoreType < VariableStoreTypeMax; StoreType++) {
    if (VariableStoreList[StoreType] == NULL) {
      DEBUG ((DEBUG_INFO, "Ranbir: L%d StoreType = %d is NULL\n", __LINE__, StoreType));
      continue;
    }

    Variable.StartPtr = GetStartPointer (VariableStoreList[StoreType]);
    Variable.EndPtr   = GetEndPointer (VariableStoreList[StoreType]);
    Variable.Volatile = (BOOLEAN)(StoreType == VariableStoreTypeVolatile);

    /*DEBUG ((DEBUG_INFO, "Ranbir: L%d StoreType = %d initiating memdump\n", __LINE__, StoreType));
    if ((Variable.EndPtr - Variable.StartPtr) > 0)
      memdump(Variable.StartPtr, Variable.EndPtr - Variable.StartPtr + 1, "ALL");
    else
      memdump(Variable.StartPtr, Variable.StartPtr - Variable.EndPtr + 1, "ALL");*/

    Status = FindVariableEx (VariableName, VendorGuid, FALSE, &Variable, AuthFormat);
    DEBUG ((DEBUG_INFO, "Ranbir: GNVNI Status1 L%d = %r StoreType = %d\n", __LINE__, Status, StoreType));
    DEBUG ((DEBUG_INFO, "Ranbir: Variable.StartPtr = 0x%x Variable.CurrPtr = 0x%x Variable.EndPtr = 0x%x\n", Variable.StartPtr, Variable.CurrPtr, Variable.EndPtr));
    if (!EFI_ERROR (Status)) {
      break;
    }
  }

  if ((Variable.CurrPtr == NULL) || EFI_ERROR (Status)) {
    //
    // For VariableName is an empty string, FindVariableEx() will try to find and return
    // the first qualified variable, and if FindVariableEx() returns error (EFI_NOT_FOUND)
    // as no any variable is found, still go to return the error (EFI_NOT_FOUND).
    //
    if (VariableName[0] != 0) {
      //
      // For VariableName is not an empty string, and FindVariableEx() returns error as
      // VariableName and VendorGuid are not a name and GUID of an existing variable,
      // there is no way to get next variable, follow spec to return EFI_INVALID_PARAMETER.
      //
      Status = EFI_INVALID_PARAMETER;
      DEBUG ((DEBUG_INFO, "Ranbir: GNVNI Status1 L%d = %r ", __LINE__, Status)); // Not coming here
    }

    DEBUG ((DEBUG_INFO, "Ranbir: GNVNI Status1 L%d = %r\n", __LINE__, Status));
    goto Done;
  }

  if (VariableName[0] != 0) {
    //
    // If variable name is not empty, get next variable.
    //
    Variable.CurrPtr = GetNextVariablePtr (Variable.CurrPtr, AuthFormat);
    DEBUG ((DEBUG_INFO, "Ranbir: GNVNI Status1 L%d = %r\n", __LINE__, Status));
  }

  while (TRUE) {
    //
    // Switch to the next variable store if needed
    //
    DEBUG ((DEBUG_INFO, "Ranbir: Checking IsValidVariableHeader L%d LoopCount = %d Variable.CurrPtr = 0x%x\n", __LINE__, LoopCount, Variable.CurrPtr));
    while (!IsValidVariableHeader (Variable.CurrPtr, Variable.EndPtr)) {
      DEBUG ((DEBUG_INFO, "Ranbir: L%d LoopCount = %d\n", __LINE__, LoopCount));
      LoopCount++;
      //
      // Find current storage index
      //
      for (StoreType = (VARIABLE_STORE_TYPE)0; StoreType < VariableStoreTypeMax; StoreType++) {
        DEBUG ((DEBUG_INFO, "Ranbir: GNVNI StoreType L%d = %d LoopCount = %d\n", __LINE__, StoreType, LoopCount));
        if ((VariableStoreList[StoreType] != NULL) && (Variable.StartPtr == GetStartPointer (VariableStoreList[StoreType]))) {
          DEBUG ((DEBUG_INFO, "Ranbir: GNVNI Status1 L%d = %r LoopCount = %d\n", __LINE__, Status, LoopCount));
          break;
        }
      }

      ASSERT (StoreType < VariableStoreTypeMax);
      //
      // Switch to next storage
      //
      for (StoreType++; StoreType < VariableStoreTypeMax; StoreType++) {
        if (VariableStoreList[StoreType] != NULL) {
          DEBUG ((DEBUG_INFO, "Ranbir: GNVNI Status1 L%d = %r LoopCount = %d\n", __LINE__, Status, LoopCount));
          break;
        }
      }

      //
      // Capture the case that
      // 1. current storage is the last one, or
      // 2. no further storage
      //
      if (StoreType == VariableStoreTypeMax) {
        Status = EFI_NOT_FOUND;
        DEBUG ((DEBUG_INFO, "Ranbir: GNVNI Status1 L%d = %r LoopCount = %d\n", __LINE__, Status, LoopCount));
        goto Done;
      }

      Variable.StartPtr = GetStartPointer (VariableStoreList[StoreType]);
      Variable.EndPtr   = GetEndPointer (VariableStoreList[StoreType]);
      Variable.CurrPtr  = Variable.StartPtr;
      DEBUG ((DEBUG_INFO, "Ranbir: GNVNI Status1 L%d = %r LoopCount = %d Variable.CurrPtr = 0x%x\n", __LINE__, Status, LoopCount, Variable.CurrPtr));
    }

    //
    // Variable is found
    //
    if ((Variable.CurrPtr->State == VAR_ADDED) || (Variable.CurrPtr->State == (VAR_IN_DELETED_TRANSITION & VAR_ADDED))) {
      if (!AtRuntime () || ((Variable.CurrPtr->Attributes & EFI_VARIABLE_RUNTIME_ACCESS) != 0)) {
        if (Variable.CurrPtr->State == (VAR_IN_DELETED_TRANSITION & VAR_ADDED)) {
          //
          // If it is a IN_DELETED_TRANSITION variable,
          // and there is also a same ADDED one at the same time,
          // don't return it.
          //
          VariablePtrTrack.StartPtr = Variable.StartPtr;
          VariablePtrTrack.EndPtr   = Variable.EndPtr;
          Status                    = FindVariableEx (
                                        GetVariableNamePtr (Variable.CurrPtr, AuthFormat),
                                        GetVendorGuidPtr (Variable.CurrPtr, AuthFormat),
                                        FALSE,
                                        &VariablePtrTrack,
                                        AuthFormat
                                        );
          if (!EFI_ERROR (Status) && (VariablePtrTrack.CurrPtr->State == VAR_ADDED)) {
            Variable.CurrPtr = GetNextVariablePtr (Variable.CurrPtr, AuthFormat);
            continue;
          }
        }

        //
        // Don't return NV variable when HOB overrides it
        //
        if ((VariableStoreList[VariableStoreTypeHob] != NULL) && (VariableStoreList[VariableStoreTypeNv] != NULL) &&
            (Variable.StartPtr == GetStartPointer (VariableStoreList[VariableStoreTypeNv]))
            )
        {
          VariableInHob.StartPtr = GetStartPointer (VariableStoreList[VariableStoreTypeHob]);
          VariableInHob.EndPtr   = GetEndPointer (VariableStoreList[VariableStoreTypeHob]);
          Status                 = FindVariableEx (
                                     GetVariableNamePtr (Variable.CurrPtr, AuthFormat),
                                     GetVendorGuidPtr (Variable.CurrPtr, AuthFormat),
                                     FALSE,
                                     &VariableInHob,
                                     AuthFormat
                                     );
          if (!EFI_ERROR (Status)) {
            Variable.CurrPtr = GetNextVariablePtr (Variable.CurrPtr, AuthFormat);
            continue;
          }
        }

        *VariablePtr = Variable.CurrPtr;
        Status       = EFI_SUCCESS;
        DEBUG ((DEBUG_INFO, "Ranbir: GNVNI Status1 L%d = %r LoopCount = %d Variable.CurrPtr = 0x%x\n", __LINE__, Status, LoopCount, Variable.CurrPtr));
        goto Done;
      }
    }

    Variable.CurrPtr = GetNextVariablePtr (Variable.CurrPtr, AuthFormat);
  }

Done:
  return Status;
}
#else
{
  EFI_STATUS  Status;

  Status = EFI_NOT_FOUND;

  // Check if the variable exists in the given variable store list
  /*DEBUG ((DEBUG_INFO, "Ranbir: L%d StoreType = %d initiating memdump\n", __LINE__, StoreType));
  if ((Variable.EndPtr - Variable.StartPtr) > 0)
    memdump(Variable.StartPtr, Variable.EndPtr - Variable.StartPtr + 1, "ALL");
  else
    memdump(Variable.StartPtr, Variable.StartPtr - Variable.EndPtr + 1, "ALL");*/

  mVarCurr = 0;
  Status = FindVariableRS (VariableName, VendorGuid);
  DEBUG ((DEBUG_INFO,
          "Ranbir: GNVNI Status1 L%d = %r CurrPtr = %d mVarCount = %d\n",
          __LINE__, Status, mVarCurr, mVarCount));

  if (EFI_ERROR (Status)) {
    //
    // For VariableName is an empty string, FindVariableEx() will try to find and return
    // the first qualified variable, and if FindVariableEx() returns error (EFI_NOT_FOUND)
    // as no any variable is found, still go to return the error (EFI_NOT_FOUND).
    //
    if (VariableName[0] != 0) {
      //
      // For VariableName is not an empty string, and FindVariableEx() returns error as
      // VariableName and VendorGuid are not a name and GUID of an existing variable,
      // there is no way to get next variable, follow spec to return EFI_INVALID_PARAMETER.
      //
      Status = EFI_INVALID_PARAMETER;
      DEBUG ((DEBUG_INFO, "Ranbir: GNVNI Status1 L%d = %r ", __LINE__, Status)); // Not coming here
    }

    DEBUG ((DEBUG_INFO, "Ranbir: GNVNI Status1 L%d = %r\n", __LINE__, Status));
    goto Done;
  }

  if (VariableName[0] != 0) {
    //
    // If variable name is not empty, get next variable.
    //
    mVarCurr++;
    if (mVarCurr >= mVarCount)
      Status = EFI_NOT_FOUND;

    DEBUG ((DEBUG_INFO, "Ranbir: GNVNI Status1 L%d = %r\n", __LINE__, Status));
    goto Done;
  }

Done:
  return Status;
}
#endif

/**
  Routine used to track statistical information about variable usage.
  The data is stored in the EFI system table so it can be accessed later.
  VariableInfo.efi can dump out the table. Only Boot Services variable
  accesses are tracked by this code. The PcdVariableCollectStatistics
  build flag controls if this feature is enabled.

  A read that hits in the cache will have Read and Cache true for
  the transaction. Data is allocated by this routine, but never
  freed.

  @param[in]      VariableName   Name of the Variable to track.
  @param[in]      VendorGuid     Guid of the Variable to track.
  @param[in]      Volatile       TRUE if volatile FALSE if non-volatile.
  @param[in]      Read           TRUE if GetVariable() was called.
  @param[in]      Write          TRUE if SetVariable() was called.
  @param[in]      Delete         TRUE if deleted via SetVariable().
  @param[in]      Cache          TRUE for a cache hit.
  @param[in,out]  VariableInfo   Pointer to a pointer of VARIABLE_INFO_ENTRY structures.

**/
VOID
UpdateVariableInfo (
  IN  CHAR16                  *VariableName,
  IN  EFI_GUID                *VendorGuid,
  IN  BOOLEAN                 Volatile,
  IN  BOOLEAN                 Read,
  IN  BOOLEAN                 Write,
  IN  BOOLEAN                 Delete,
  IN  BOOLEAN                 Cache,
  IN OUT VARIABLE_INFO_ENTRY  **VariableInfo
  )
{
  VARIABLE_INFO_ENTRY  *Entry;

  if (FeaturePcdGet (PcdVariableCollectStatistics)) {
    if ((VariableName == NULL) || (VendorGuid == NULL) || (VariableInfo == NULL)) {
      return;
    }

    if (AtRuntime ()) {
      // Don't collect statistics at runtime.
      return;
    }

    if (*VariableInfo == NULL) {
      //
      // On the first call allocate a entry and place a pointer to it in
      // the EFI System Table.
      //
      *VariableInfo = AllocateZeroPool (sizeof (VARIABLE_INFO_ENTRY));
      ASSERT (*VariableInfo != NULL);

      CopyGuid (&(*VariableInfo)->VendorGuid, VendorGuid);
      (*VariableInfo)->Name = AllocateZeroPool (StrSize (VariableName));
      ASSERT ((*VariableInfo)->Name != NULL);
      StrCpyS ((*VariableInfo)->Name, StrSize (VariableName)/sizeof (CHAR16), VariableName);
      (*VariableInfo)->Volatile = Volatile;
    }

    for (Entry = (*VariableInfo); Entry != NULL; Entry = Entry->Next) {
      if (CompareGuid (VendorGuid, &Entry->VendorGuid)) {
        if (StrCmp (VariableName, Entry->Name) == 0) {
          if (Read) {
            Entry->ReadCount++;
          }

          if (Write) {
            Entry->WriteCount++;
          }

          if (Delete) {
            Entry->DeleteCount++;
          }

          if (Cache) {
            Entry->CacheCount++;
          }

          return;
        }
      }

      if (Entry->Next == NULL) {
        //
        // If the entry is not in the table add it.
        // Next iteration of the loop will fill in the data.
        //
        Entry->Next = AllocateZeroPool (sizeof (VARIABLE_INFO_ENTRY));
        ASSERT (Entry->Next != NULL);

        CopyGuid (&Entry->Next->VendorGuid, VendorGuid);
        Entry->Next->Name = AllocateZeroPool (StrSize (VariableName));
        ASSERT (Entry->Next->Name != NULL);
        StrCpyS (Entry->Next->Name, StrSize (VariableName)/sizeof (CHAR16), VariableName);
        Entry->Next->Volatile = Volatile;
      }
    }
  }
}
