// Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cgpt.h"

#include <getopt.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fs.h>

#include "cgpt_params.h"

static void Usage(void)
{
  printf("\nUsage: %s add [OPTIONS] DRIVE\n\n"
         "Add, edit, or remove a partition entry.\n\n"
         "Options:\n"
         "  -i NUM       Specify partition (default is next available)\n"
         "  -b NUM       Beginning sector\n"
         "  -s NUM       Size in sectors\n"
         "  -t GUID      Partition Type GUID\n"
         "  -u GUID      Partition Unique ID\n"
         "  -l LABEL     Label\n"
         "  -S NUM       set Successful flag (0|1)\n"
         "  -T NUM       set Tries flag (0-15)\n"
         "  -P NUM       set Priority flag (0-15)\n"
         "  -A NUM       set raw 64-bit attribute value\n"
         "\n"
         "Use the -i option to modify an existing partition.\n"
         "The -b, -s, and -t options must be given for new partitions.\n"
         "\n", progname);
  PrintTypes();
}

static unsigned long long _lba_count(const char *blk_device)
{
    unsigned long long numblocks = 0;
    int fd;

    fd = open(blk_device, O_RDONLY);
    if (fd != -1) {
      ioctl(fd, BLKGETSIZE64, &numblocks);
      close(fd);
    }
    return numblocks/512;
}

static int parse_lba_end(const char *line, unsigned long long *lba_end)
{
        unsigned long long lba_sub;

	*lba_end = 0;

	line = strstr(line, "$calc(");
	if (!line)
	        return -1;

	if (1 != sscanf(line,"$calc($lba_end-%llu)", &lba_sub))
                return -1;

	*lba_end = lba_sub;

	return 0;
}

int cmd_add(int argc, char *argv[]) {

  CgptAddParams params;
  memset(&params, 0, sizeof(params));

  int c;
  int errorcnt = 0;
  char *e = 0;
  unsigned long long lba_end;

  opterr = 0;                     // quiet, you
  while ((c=getopt(argc, argv, ":hi:b:s:t:u:l:S:T:P:A:")) != -1)
  {
    switch (c)
    {
    case 'i':
      params.partition = (uint32_t)strtoul(optarg, &e, 0);
      if (!*optarg || (e && *e))
      {
        Error("invalid argument to -%c: \"%s\"\n", c, optarg);
        errorcnt++;
      }
      break;
    case 'b':
      params.set_begin = 1;
      params.begin = strtoull(optarg, &e, 0);
      if (!*optarg || (e && *e))
      {
        Error("invalid argument to -%c: \"%s\"\n", c, optarg);
        errorcnt++;
      }
      break;
    case 's':
      params.set_size = 1;
      if (!parse_lba_end(optarg, &lba_end)) {
        params.size = ( _lba_count(argv[argc-1] ) - params.begin) - lba_end ;
      } else
         params.size = strtoull(optarg, &e, 0);

      if (!*optarg || (e && *e))
      {
        Error("invalid argument to -%c: \"%s\"\n", c, optarg);
        errorcnt++;
      }
      break;
    case 't':
      params.set_type = 1;
      if (CGPT_OK != SupportedType(optarg, &params.type_guid) &&
          CGPT_OK != StrToGuid(optarg, &params.type_guid)) {
        Error("invalid argument to -%c: %s\n", c, optarg);
        errorcnt++;
      }
      break;
    case 'u':
      params.set_unique = 1;
      if (CGPT_OK != StrToGuid(optarg, &params.unique_guid)) {
        Error("invalid argument to -%c: %s\n", c, optarg);
        errorcnt++;
      }
      break;
    case 'l':
      params.label = optarg;
      break;
    case 'S':
      params.set_successful = 1;
      params.successful = (uint32_t)strtoul(optarg, &e, 0);
      if (!*optarg || (e && *e))
      {
        Error("invalid argument to -%c: \"%s\"\n", c, optarg);
        errorcnt++;
      }
      if (params.successful < 0 || params.successful > 1) {
        Error("value for -%c must be between 0 and 1", c);
        errorcnt++;
      }
      break;
    case 'T':
      params.set_tries = 1;
      params.tries = (uint32_t)strtoul(optarg, &e, 0);
      if (!*optarg || (e && *e))
      {
        fprintf(stderr, "%s: invalid argument to -%c: \"%s\"\n",
                progname, c, optarg);
        errorcnt++;
      }
      if (params.tries < 0 || params.tries > 15) {
        Error("value for -%c must be between 0 and 15", c);
        errorcnt++;
      }
      break;
    case 'P':
      params.set_priority = 1;
      params.priority = (uint32_t)strtoul(optarg, &e, 0);
      if (!*optarg || (e && *e))
      {
        Error("invalid argument to -%c: \"%s\"\n", c, optarg);
        errorcnt++;
      }
      if (params.priority < 0 || params.priority > 15) {
        Error("value for -%c must be between 0 and 15", c);
        errorcnt++;
      }
      break;
    case 'A':
      params.set_raw = 1;
      params.raw_value = strtoull(optarg, &e, 0);
      if (!*optarg || (e && *e))
      {
        Error("invalid argument to -%c: \"%s\"\n", c, optarg);
        errorcnt++;
      }
      break;

    case 'h':
      Usage();
      return CGPT_OK;
    case '?':
      Error("unrecognized option: -%c\n", optopt);
      errorcnt++;
      break;
    case ':':
      Error("missing argument to -%c\n", optopt);
      errorcnt++;
      break;
    default:
      errorcnt++;
      break;
    }
  }
  if (errorcnt)
  {
    Usage();
    return CGPT_FAILED;
  }

  if (optind >= argc)
  {
    Error("missing drive argument\n");
    return CGPT_FAILED;
  }

  params.drive_name = argv[optind];

  return cgpt_add(&params);
}
