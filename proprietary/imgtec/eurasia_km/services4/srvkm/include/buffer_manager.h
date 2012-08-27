/**********************************************************************
 *
 * Copyright (C) Imagination Technologies Ltd. All rights reserved.
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope it will be useful but, except 
 * as otherwise stated in writing, without any warranty; without even the 
 * implied warranty of merchantability or fitness for a particular purpose. 
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 * 
 * The full GNU General Public License is included in this distribution in
 * the file called "COPYING".
 *
 * Contact Information:
 * Imagination Technologies Ltd. <gpl-support@imgtec.com>
 * Home Park Estate, Kings Langley, Herts, WD4 8LZ, UK 
 *
 ******************************************************************************/

#ifndef _BUFFER_MANAGER_H_
#define _BUFFER_MANAGER_H_

#include "img_types.h"
#include "ra.h"
#include "perproc.h"

#if defined(__cplusplus)
extern "C"{
#endif

typedef struct _BM_HEAP_ BM_HEAP;

struct _BM_MAPPING_
{
	enum
	{
		hm_wrapped = 1,		
		hm_wrapped_scatter,	
		hm_wrapped_virtaddr, 
		hm_wrapped_scatter_virtaddr, 
		hm_env,				
		hm_contiguous		
	} eCpuMemoryOrigin;

	BM_HEAP				*pBMHeap;	
	RA_ARENA			*pArena;	

	IMG_CPU_VIRTADDR	CpuVAddr;
	IMG_CPU_PHYADDR		CpuPAddr;
	IMG_DEV_VIRTADDR	DevVAddr;
	IMG_SYS_PHYADDR		*psSysAddr;
	IMG_SIZE_T			uSize;
    IMG_HANDLE          hOSMemHandle;
	IMG_UINT32			ui32Flags;

	/* GPU mapping reference count
	 * When goes down to 0 GPU mapping
	 * gets removed */
	IMG_UINT32			ui32MappingCount;

	/* need to track the original required alignment to make sure
	 * that an unmapped buffer which is later remapped to device
	 * is remapped with the original alignment restrictions.
	 */
	IMG_UINT32			ui32DevVAddrAlignment;
};

typedef struct _BM_BUF_
{
	IMG_CPU_VIRTADDR	*CpuVAddr;
    IMG_VOID            *hOSMemHandle;
	IMG_CPU_PHYADDR		CpuPAddr;
	IMG_DEV_VIRTADDR	DevVAddr;

	BM_MAPPING			*pMapping;
	IMG_UINT32			ui32RefCount;
	IMG_UINT32			ui32ExportCount;
} BM_BUF;

struct _BM_HEAP_
{
	IMG_UINT32				ui32Attribs;
	BM_CONTEXT				*pBMContext;
	RA_ARENA				*pImportArena;
	RA_ARENA				*pLocalDevMemArena;
	RA_ARENA				*pVMArena;
	DEV_ARENA_DESCRIPTOR	sDevArena;
	MMU_HEAP				*pMMUHeap;
	PDUMP_MMU_ATTRIB 		*psMMUAttrib;
	
	struct _BM_HEAP_ 		*psNext;
	struct _BM_HEAP_ 		**ppsThis;
	
	IMG_UINT32 				ui32XTileStride;
};

struct _BM_CONTEXT_
{
	MMU_CONTEXT	*psMMUContext;

	
	 BM_HEAP *psBMHeap;

	
	 BM_HEAP *psBMSharedHeap;

	PVRSRV_DEVICE_NODE *psDeviceNode;

	
	HASH_TABLE *pBufferHash;

	
	IMG_HANDLE hResItem;

	IMG_UINT32 ui32RefCount;

	

	struct _BM_CONTEXT_ *psNext;
	struct _BM_CONTEXT_ **ppsThis;
};

typedef struct _XPROC_DATA_{
	IMG_UINT32 ui32RefCount;
	IMG_UINT32 ui32AllocFlags;
	IMG_UINT32 ui32Size;
	IMG_UINT32 ui32PageSize;
    RA_ARENA *psArena;
    IMG_SYS_PHYADDR sSysPAddr;
	IMG_VOID *pvCpuVAddr;
	IMG_HANDLE hOSMemHandle;
	IMG_UINT32 ui32Offsets[PVRSRV_MAX_NUMBER_OF_MM_BUFFER_PLANES];
} XPROC_DATA;

extern XPROC_DATA gXProcWorkaroundShareData[];
typedef IMG_VOID *BM_HANDLE;

#define BP_POOL_MASK         0x7

#define BP_CONTIGUOUS			(1 << 3)
#define BP_PARAMBUFFER			(1 << 4)

#define BM_MAX_DEVMEM_ARENAS  2

IMG_HANDLE
BM_CreateContext(PVRSRV_DEVICE_NODE			*psDeviceNode,
				 IMG_DEV_PHYADDR			*psPDDevPAddr,
				 PVRSRV_PER_PROCESS_DATA	*psPerProc,
				 IMG_BOOL					*pbCreated);


PVRSRV_ERROR
BM_DestroyContext (IMG_HANDLE hBMContext,
					IMG_BOOL *pbCreated);


IMG_HANDLE
BM_CreateHeap (IMG_HANDLE hBMContext,
				DEVICE_MEMORY_HEAP_INFO *psDevMemHeapInfo);

IMG_VOID
BM_DestroyHeap (IMG_HANDLE hDevMemHeap);


IMG_BOOL
BM_Reinitialise (PVRSRV_DEVICE_NODE *psDeviceNode);

IMG_BOOL
BM_Alloc (IMG_HANDLE			hDevMemHeap,
			IMG_DEV_VIRTADDR	*psDevVAddr,
			IMG_SIZE_T			uSize,
			IMG_UINT32			*pui32Flags,
			IMG_UINT32			uDevVAddrAlignment,
			IMG_PVOID			pvPrivData,
			IMG_UINT32			ui32PrivDataLength,
			BM_HANDLE			*phBuf);

IMG_BOOL
BM_Wrap (	IMG_HANDLE hDevMemHeap,
		    IMG_SIZE_T ui32Size,
			IMG_SIZE_T ui32Offset,
			IMG_BOOL bPhysContig,
			IMG_SYS_PHYADDR *psSysAddr,
			IMG_VOID *pvCPUVAddr,
			IMG_UINT32 *pui32Flags,
			BM_HANDLE *phBuf);

IMG_VOID
BM_Free (BM_HANDLE hBuf,
		IMG_UINT32 ui32Flags);


IMG_CPU_VIRTADDR
BM_HandleToCpuVaddr (BM_HANDLE hBuf);

IMG_DEV_VIRTADDR
BM_HandleToDevVaddr (BM_HANDLE hBuf);

IMG_SYS_PHYADDR
BM_HandleToSysPaddr (BM_HANDLE hBuf);

IMG_HANDLE
BM_HandleToOSMemHandle (BM_HANDLE hBuf);

IMG_INT32
BM_RemapToDev(BM_HANDLE hBuf);

IMG_INT32
BM_UnmapFromDev(BM_HANDLE hBuf);


IMG_VOID BM_GetPhysPageAddr(PVRSRV_KERNEL_MEM_INFO *psMemInfo,
								IMG_DEV_VIRTADDR sDevVPageAddr,
								IMG_DEV_PHYADDR *psDevPAddr);

MMU_CONTEXT* BM_GetMMUContext(IMG_HANDLE hDevMemHeap);

MMU_CONTEXT* BM_GetMMUContextFromMemContext(IMG_HANDLE hDevMemContext);

IMG_HANDLE BM_GetMMUHeap(IMG_HANDLE hDevMemHeap);

PVRSRV_DEVICE_NODE* BM_GetDeviceNode(IMG_HANDLE hDevMemContext);


IMG_HANDLE BM_GetMappingHandle(PVRSRV_KERNEL_MEM_INFO *psMemInfo);

IMG_VOID BM_Export(BM_HANDLE hBuf);

IMG_VOID BM_FreeExport(BM_HANDLE hBuf, IMG_UINT32 ui32Flags);

PVRSRV_ERROR BM_XProcWorkaroundSetShareIndex(IMG_UINT32 ui32Index);
PVRSRV_ERROR BM_XProcWorkaroundUnsetShareIndex(IMG_UINT32 ui32Index);
PVRSRV_ERROR BM_XProcWorkaroundFindNewBufferAndSetShareIndex(IMG_UINT32 *pui32Index);
IMG_INT32 BM_XProcGetShareDataRefCount(IMG_UINT32 ui32Index);

static INLINE IMG_CHAR *
_BMMappingType (IMG_INT eCpuMemoryOrigin)
{
	switch (eCpuMemoryOrigin)
	{
	case hm_wrapped: return "hm_wrapped";
	case hm_wrapped_scatter: return "hm_wrapped_scatter";
	case hm_wrapped_virtaddr: return "hm_wrapped_virtaddr";
	case hm_wrapped_scatter_virtaddr: return "hm_wrapped_scatter_virtaddr";
	case hm_env: return "hm_env";
	case hm_contiguous: return "hm_contiguous";
	}
	return "junk";
}

#if defined(PVRSRV_REFCOUNT_DEBUG)
IMG_VOID _BM_XProcIndexAcquireDebug(const IMG_CHAR *pszFile, IMG_INT iLine, IMG_UINT32 ui32Index);
IMG_VOID _BM_XProcIndexReleaseDebug(const IMG_CHAR *pszFile, IMG_INT iLine, IMG_UINT32 ui32Index);

#define BM_XProcIndexAcquire(x...) \
	_BM_XProcIndexAcquireDebug(__FILE__, __LINE__, x)
#define BM_XProcIndexRelease(x...) \
	_BM_XProcIndexReleaseDebug(__FILE__, __LINE__, x)

#else
IMG_VOID _BM_XProcIndexAcquire(IMG_UINT32 ui32Index);
IMG_VOID _BM_XProcIndexRelease(IMG_UINT32 ui32Index);

#define BM_XProcIndexAcquire(x...) \
	_BM_XProcIndexAcquire( x)
#define BM_XProcIndexRelease(x...) \
	_BM_XProcIndexRelease( x)
#endif

#if defined(__cplusplus)
}
#endif

#endif

