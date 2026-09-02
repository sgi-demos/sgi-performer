#ifndef	__EVENTOR_H__
#define	__EVENTOR_H__

#ifndef	WIN32
#include <sys/time.h>
#else
#include <windows.h>
#endif

#ifndef WIN32
#include <inttypes.h>
#else
#include <wintypes.h>
#endif

/*
 *	data types and constants
 */

enum
{
	EV_VER0_9,
	EV_VER1_0,
	EV_VER1_1,
	EV_VER1_2
};

#define	EVMAGIC_VER1_0			0x0250
#define	EVMAGIC_VER1_1			0x0251
#define	EVMAGIC_VER1_2			0x0252
#define	EVMAGIC					EVMAGIC_VER1_2

#define	EVTYPE_GRAPHIC			0x00000001
#define	EVTYPE_GRAPHIC_PROF		0x00000002
#define	EVTYPE_FRAME			0x00000004

#define	EVDBG_MODE_PIPEFLUSH	0x00000001
#define	EVDBG_MODE_GLPROF		0x00000002
#define	EVDBG_MODE_GLDEBUG		0x00000004

#define	EVGL_IRISGL				1
#define	EVGL_OPENGL				2

typedef void *(*evMallocFunc) (size_t);
typedef void (*evFreeFunc) (void*);

typedef	struct timeval	TIMEVAL;


enum
{
	EV_ITYPE_INT,
	EV_ITYPE_FLOAT,
	EV_ITYPE_ENUM
};


/*
 *	Old disk structures
 */
typedef struct
{
	int			eventClass;
	TIMEVAL		start, end;
} evEvent_DiskV1_0;

typedef struct
{
	char		*name;
	int			fractionSize;
} evInfo_DiskV1_1;

/*
 *	Data types
 */
typedef struct
{
	TIMEVAL		start, end;
	int			eventClass;
	int			pid;

	int			*info;
} evEvent;

typedef struct _evInfo
{
	char		*name;
	int			fractionSize;
	int			type;
	int			enumId;
} evInfo;

typedef struct evClassData
{

	char		*name;
	int			type;

	short		groupLevel;
	uint64_t	groupMask;

	int			infoSize;

	evInfo		*infoDescription;
} evClassData;

typedef struct
{
	char		*name;

	int			nitems;
	char		**items;
} evEnum;

typedef struct
{
	char		*name;
	int			from, to;
} evRange;

typedef struct
{
	int			pid;
	char		*name;
} evProcess;

typedef struct
{
	uint64_t	mask;
	char		*name;
	int			initState;
} evGroup;

typedef struct evEventList
{
	char				*name;

	int					maxEvents;
	int					numEvents;
	int					maxClasses;

	int					numEnums;
	int					numRanges;
	int					numProcesses;
	int					numGroups;


	int					lock;
	evEvent				**openEvents;

	evClassData			*classData;

	evEvent				*events;

	evEnum				*enums;

	evRange				*ranges;
	evProcess			*procs;
	evGroup				*groups;

	void				*userData;
} evEventList;

/*
 *	prototypes
 */

#ifdef __cplusplus
extern "C" 
{
#endif


void evSetMallocFunc (evMallocFunc func);
void evSetFreeFunc (evFreeFunc func);
int evGetMyId (void);
void evRegisterList (evEventList *list);
evEventList *evCreateList (int maxEvents, int maxClasses);
void evListName (evEventList *list, char *name);
void evFreeList (evEventList *list);
void evClearList (evEventList *list);
void evClearAll (evEventList *list);
void evClearAllLists();

void evClassName (evEventList *list, int eventClass, char *name);
void evClassType (evEventList *list, int eventClass, int type);
void evClassGroupMask (evEventList *list, int eventClass, uint64_t mask);
void evClassGroupLevel (evEventList *list, int eventClass, int groupLevel);
void evClassGroupName (evEventList *list, int eventClass, char *name);
void evClassInfoSize (evEventList *list, int eventClass, int info_size);
void evInfoDescription (evEventList *list, int eventClass, 
							int info, char *name, int fractionSize);
void evInfoType (evEventList *list, int eventClass, int info, int type);
void evInfoName (evEventList *list, int eventClass, int info, char *name);
void evInfoFraction (evEventList *list, int eventClass,
					int info, int fractionSize);
void evInfoEnum (evEventList *list, int eventClass, int info, char *name);

void evSetEnum (evEventList *list, char *name, int nitems, char **items);

void evSetProc (evEventList *list, int _pid, char *name);

void evSetGroup (evEventList *list, uint64_t mask, char *name, int initState);
int evGetGroupId (evEventList *list, char *name);

void evConfig (evEventList *list);

void evListUserData (evEventList *list, void *data);
void *evGetListUserData(evEventList *list);
void evGetAllLists (evEventList ***lists, int *num);

evEvent *evStart (evEventList *list, int eventClass);
void evEnd (evEventList *list, evEvent *event);

evEvent *evFrame (evEventList *list, int eventClass);

evEvent *evStartT (evEventList *list, int eventClass, TIMEVAL *time);
void evEndT (evEventList *list, evEvent *event, TIMEVAL *time);

void evSetInfo (evEventList *list, evEvent *event, int index, int info);

void evWriteList (char *file, evEventList *list);
evEventList *evReadList (char *file);
void evWriteAllLists (char *file);
evEventList *evMergeLists (int num, evEventList **lists);

void evPrintTotals (FILE *fp, evEventList *list);

int evGetEventClassId(evEventList *list, char *name);
int evGetInfoId(evEventList *list, int eventClass, char *name);
int evGetFreeClassId (evEventList *list);

#ifdef __cplusplus
}
#endif

#endif
