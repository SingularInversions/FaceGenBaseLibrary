/*
 * Program: IPTCUTIL.C
 *
 * Purpose: Convert between IPTC binary and a "special" IPTC text file format.
 *
 * Usage:   iptcutil -t | -b [-i file] [-o file] <input >output
 *
 * Notes:   You tell the program the "type" of input file via the -t and -b
 *          switches. The -t says that the input is text, while the -b says
 *          that the input is binary IPTC. You can use either the -i or the
 *          -o switches to tell the program what the input and output files
 *          will be, or use simple piping.
 *
 * Author:  William T. Radcliffe (billr@corbis.com)
 *          Parts of this program were derived from other places. The original
 *          binary to text conversion was taken from the PHP distribution and
 *          the tokenizer was written many years ago, by someone else as well.
 *
 * This software is provided freely "as is", without warranty of any kind,
 * express or implied, including but not limited to the warranties of
 * merchantability, fitness for a particular purpose and noninfringement.
 * In no event shall William T. Radcliffe be liable for any claim, damages or
 * other liability, whether in an action of contract, tort or otherwise,
 * arising from, out of or in connection with IPTCUTIL
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#ifdef _VISUALC_
#include <io.h>
#endif

typedef struct _tag_spec
{
  short
    id;

  char
    *name;
} tag_spec;

static tag_spec tags[] = {
  { 5, (char *) "Image Name" },
  { 7, (char *) "Edit Status" },
  { 10, (char *) "Priority" },
  { 15, (char *) "Category" },
  { 20, (char *) "Supplemental Category" },
  { 22, (char *) "Fixture Identifier" },
  { 25, (char *) "Keyword" },
  { 30, (char *) "Release Date" },
  { 35, (char *) "Release Time" },
  { 40, (char *) "Special Instructions" },
  { 45, (char *) "Reference Service" },
  { 47, (char *) "Reference Date" },
  { 50, (char *) "Reference Number" },
  { 55, (char *) "Created Date" },
  { 60, (char *) "Created Time" },
  { 65, (char *) "Originating Program" },
  { 70, (char *) "Program Version" },
  { 75, (char *) "Object Cycle" },
  { 80, (char *) "Byline" },
  { 85, (char *) "Byline Title" },
  { 90, (char *) "City" },
  { 95, (char *) "Province State" },
  { 100, (char *) "Country Code" },
  { 101, (char *) "Country" },
  { 103, (char *) "Original Transmission Reference" },
  { 105, (char *) "Headline" },
  { 110, (char *) "Credit" },
  { 115, (char *) "Source" },
  { 116, (char *) "Copyright String" },
  { 120, (char *) "Caption" },
  { 121, (char *) "Local Caption" },
  { 122, (char *) "Caption Writer" },
  { 200, (char *) "Custom Field 1" },
  { 201, (char *) "Custom Field 2" },
  { 202, (char *) "Custom Field 3" },
  { 203, (char *) "Custom Field 4" },
  { 204, (char *) "Custom Field 5" },
  { 205, (char *) "Custom Field 6" },
  { 206, (char *) "Custom Field 7" },
  { 207, (char *) "Custom Field 8" },
  { 208, (char *) "Custom Field 9" },
  { 209, (char *) "Custom Field 10" },
  { 210, (char *) "Custom Field 11" },
  { 211, (char *) "Custom Field 12" },
  { 212, (char *) "Custom Field 13" },
  { 213, (char *) "Custom Field 14" },
  { 214, (char *) "Custom Field 15" },
  { 215, (char *) "Custom Field 16" },
  { 216, (char *) "Custom Field 17" },
  { 217, (char *) "Custom Field 18" },
  { 218, (char *) "Custom Field 19" },
  { 219, (char *) "Custom Field 20" }
};

int stringnicmp(const char *p,const char *q,size_t n)
{
  register int
    i,
    j;

  if (p == q)
    return(0);
  if (p == (char *) NULL)
    return(-1);
  if (q == (char *) NULL)
    return(1);
  while ((*p != '\0') && (*q != '\0'))
  {
    if ((*p == '\0') || (*q == '\0'))
      break;
    i=(*p);
    if (islower(i))
      i=toupper(i);
    j=(*q);
    if (islower(j))
      j=toupper(j);
    if (i != j)
      break;
    n--;
    if (n == 0)
      break;
    p++;
    q++;
  }
  return(toupper(*p)-toupper(*q));
}

void writeLong(FILE *ofile, long val)
{
  fputc((val >> 24) & 255, ofile);
  fputc((val >> 16) & 255, ofile);
  fputc((val >>  8) & 255, ofile);
  fputc(val & 255, ofile);
}

void writeWord(FILE *ofile, int val)
{
  fputc((val >>  8) & 255, ofile);
  fputc(val & 255, ofile);
}

long readLong(FILE *ifile)
{
  unsigned char
    buffer[4];

  int
    i,
    c;

  for (i=0; i<4; i++)
  {
    c =  getc(ifile);
    if (c == EOF) return -1;
    buffer[i] = c;
  }
  return (((long) buffer[ 0 ]) << 24) |
         (((long) buffer[ 1 ]) << 16) | 
	       (((long) buffer[ 2 ]) <<  8) |
         (((long) buffer[ 3 ]));
}

int readWord(FILE *ifile)
{
  unsigned char
    buffer[2];

  int
    i,
    c;

  for (i=0; i<2; i++)
  {
    c =  getc(ifile);
    if (c == EOF) return -1;
    buffer[i] = c;
  }
  return (((int) buffer[ 0 ]) <<  8) |
         (((int) buffer[ 1 ]));
}

long readLongFromBuffer(char **s, long *len)
{
  unsigned char
    buffer[4];

  int
    i,
    c;

  for (i=0; i<4; i++)
  {
    c = *(*s)++; (*len)--;
    if (*len < 0) return -1;
    buffer[i] = c;
  }
  return (((long) buffer[ 0 ]) << 24) |
         (((long) buffer[ 1 ]) << 16) | 
	       (((long) buffer[ 2 ]) <<  8) |
         (((long) buffer[ 3 ]));
}

int readWordFromBuffer(char **s, long *len)
{
  unsigned char
    buffer[2];

  int
    i,
    c;

  for (i=0; i<2; i++)
  {
    c = *(*s)++; (*len)--;
    if (*len < 0) return -1;
    buffer[i] = c;
  }
  return (((int) buffer[ 0 ]) <<  8) |
         (((int) buffer[ 1 ]));
}

/*
 * We format the output using HTML conventions
 * to preserve control characters and such.
 */
void formatString(FILE *ofile, const char *s, int len)
{
  putc('"', ofile);
  for (; len > 0; --len, ++s) {
    int c = (*s) & 255;
    switch (c) {
    case '&':
      fputs("&amp;", ofile);
      break;
#ifdef HANDLE_GT_LT
    case '<':
      fputs("&lt;", ofile);
      break;
    case '>':
      fputs("&gt;", ofile);
      break;
#endif
    case '"':
      fputs("&quot;", ofile);
      break;
    default:
      if (isprint(c))
        putc(*s, ofile);
      else
        fprintf(ofile, "&#%d;", c & 255);
      break;
    }
  }
  fputs("\"\n", ofile);
}

typedef struct _html_code
{
  short
    len;
  const char
    *code,
    val;
} html_code;

static html_code html_codes[] = {
#ifdef HANDLE_GT_LT
  { 4,"&lt;",'<' },
  { 4,"&gt;",'>' },
#endif
  { 5,"&amp;",'&' },
  { 6,"&quot;",'"' },
  { 6,"&apos;",'\''}
};

/*
 * This routine converts HTML escape sequence
 * back to the original ASCII representation.
 * - returns the number of characters dropped.
 */
int convertHTMLcodes(char *s, int len)
{
  if (len <=0 || s==(char*)NULL || *s=='\0')
    return 0;

  if (s[1] == '#')
    {
      int val, o;

      if (sscanf(s,"&#%d;",&val) == 1)
      {
        o = 3;
        while (s[o] != ';')
        {
          o++;
          if (o > 5)
            break;
        }
        if (o < 6)
          strcpy(s+1, s+1+o);
        *s = val;
        return o;
      }
    }
  else
    {
      int
        i,
        codes = sizeof(html_codes) / sizeof(html_code);

      for (i=0; i < codes; i++)
      {
        if (html_codes[i].len <= len)
          if (stringnicmp(s, html_codes[i].code, html_codes[i].len) == 0)
            {
              strcpy(s+1, s+html_codes[i].len);
              *s = html_codes[i].val;
              return html_codes[i].len-1;
            }
      }
    }
  return 0;
}

int formatIPTC(FILE *ifile, FILE *ofile)
{
  unsigned int
    foundiptc,
    tagsfound;

  unsigned char
    recnum,
    dataset;

  unsigned char
    *readable,
    *str;

  long
    tagindx,
    taglen;

  int
    i,
    tagcount = sizeof(tags) / sizeof(tag_spec);

  int
    c;

  foundiptc = 0; /* found the IPTC-Header */
  tagsfound = 0; /* number of tags found */

  c = getc(ifile);
  while (c != EOF)
  {
	  if (c == 0x1c)
	    foundiptc = 1;
	  else
      {
        if (foundiptc)
	        return -1;
        else
	        continue;
	    }

    /* we found the 0x1c tag and now grab the dataset and record number tags */
    c = getc(ifile);
	  if (c == EOF) return -1;
    dataset = c;
    c = getc(ifile);
	  if (c == EOF) return -1;
    recnum = c;
    /* try to match this record to one of the ones in our named table */
    for (i=0; i< tagcount; i++)
    {
      if (tags[i].id == recnum)
          break;
    }
    if (i < tagcount)
      readable = (unsigned char *) tags[i].name;
    else
      readable = (unsigned char *) "";

    /* then we decode the length of the block that follows - long or short fmt */
    c = getc(ifile);
	  if (c == EOF) return -1;
	  if (c & (unsigned char) 0x80)
      {
        printf("Unable to read block longer then 32k");
        return 0;
      }
    else
      {
        ungetc(c, ifile);
        taglen = readWord(ifile);
	    }
    if (taglen < 0) return -1;
    /* make a buffer to hold the tag data and snag it from the input stream */
    str=(unsigned char *) malloc((unsigned int) (taglen+1));
    if (str == (unsigned char *) NULL)
      {
        printf("Memory allocation failed");
        return 0;
      }
    for (tagindx=0; tagindx<taglen; tagindx++)
    {
      c = getc(ifile);
      if (c == EOF) return -1;
      str[tagindx] = c;
    }
    str[ taglen ] = 0;

    /* now finish up by formatting this binary data into ASCII equivalent */
    if (strlen((char *)readable) > 0)
	    fprintf(ofile, "%d#%d#%s=",(unsigned int)dataset, (unsigned int) recnum, readable);
    else
	    fprintf(ofile, "%d#%d=",(unsigned int)dataset, (unsigned int) recnum);
    formatString( ofile, (char *)str, taglen );
    free(str);

	  tagsfound++;

    c = getc(ifile);
  }
  return tagsfound;
}

int formatIPTCfromBuffer(FILE *ofile, char *s, long len)
{
  unsigned int
    foundiptc,
    tagsfound;

  unsigned char
    recnum,
    dataset;

  unsigned char
    *readable,
    *str;

  long
    tagindx,
    taglen;

  int
    i,
    tagcount = sizeof(tags) / sizeof(tag_spec);

  int
    c;

  foundiptc = 0; /* found the IPTC-Header */
  tagsfound = 0; /* number of tags found */

  while (len > 0)
  {
    c = *s++; len--;
	  if (c == 0x1c)
	    foundiptc = 1;
	  else
      {
        if (foundiptc)
	        return -1;
        else
	        continue;
	    }

    /* we found the 0x1c tag and now grab the dataset and record number tags */
    c = *s++; len--;
	  if (len < 0) return -1;
    dataset = c;
    c = *s++; len--;
	  if (len < 0) return -1;
    recnum = c;
    /* try to match this record to one of the ones in our named table */
    for (i=0; i< tagcount; i++)
    {
      if (tags[i].id == recnum)
          break;
    }
    if (i < tagcount)
      readable = (unsigned char *) tags[i].name;
    else
      readable = (unsigned char *) "";

    /* then we decode the length of the block that follows - long or short fmt */
    c = *s++; len--;
	  if (len < 0) return -1;
	  if (c & (unsigned char) 0x80)
      {
        printf("Unable to read block longer then 32k");
        return 0;
      }
    else
      {
        s--; len++;
        taglen = readWordFromBuffer(&s, &len);
	    }
    if (taglen < 0) return -1;
    /* make a buffer to hold the tag data and snag it from the input stream */
    str=(unsigned char *) malloc((unsigned int) (taglen+1));
    if (str == (unsigned char *) NULL)
      {
        printf("Memory allocation failed");
        return 0;
      }
    for (tagindx=0; tagindx<taglen; tagindx++)
    {
      c = *s++; len--;
      if (len < 0) return -1;
      str[tagindx] = c;
    }
    str[ taglen ] = 0;

    /* now finish up by formatting this binary data into ASCII equivalent */
    if (strlen((char *)readable) > 0)
	    fprintf(ofile, "%d#%d#%s=",(unsigned int)dataset, (unsigned int) recnum, readable);
    else
	    fprintf(ofile, "%d#%d=",(unsigned int)dataset, (unsigned int) recnum);
    formatString( ofile, (char *)str, taglen );
    free(str);

	  tagsfound++;
  }
  return tagsfound;
}

#define IPTC_ID 1028
#define THUMBNAIL_ID 1033

int format8BIM(FILE *ifile, FILE *ofile)
{
  unsigned int
    foundOSType;

  long
    Size;

  int
    ID,
    resCount,
    i,
    c;

  unsigned char
    *PString,
    *str;

  resCount = foundOSType = 0; /* found the OSType */

  c = getc(ifile);
  while (c != EOF)
  {
	  if (c == '8')
      {
        unsigned char
          buffer[5];

        buffer[0] = c;
        for (i=1; i<4; i++)
        {
          c =  getc(ifile);
          if (c == EOF) return -1;
          buffer[i] = c;
        }
        buffer[4] = 0;
        if (strcmp((const char *)buffer, "8BIM") == 0)
          foundOSType = 1;
        else
          continue;
	    }
	  else
      {
        c = getc(ifile);
	      continue;
      }

    /* we found the OSType (8BIM) and now grab the ID, PString, and Size fields */
    ID = readWord(ifile);
    if (ID < 0) return -1;
    {
      unsigned char
        plen;

      c =  getc(ifile);
      if (c == EOF) return -1;
      plen = c;
      PString=(unsigned char *) malloc((unsigned int) (plen+1));
      if (PString == (unsigned char *) NULL)
      {
        printf("Memory allocation failed");
        return 0;
      }
      for (i=0; i<plen; i++)
      {
        c =  getc(ifile);
        if (c == EOF) return -1;
        PString[i] = c;
      }
      PString[ plen ] = 0;
      if (!(plen&1)) 
      {
        c =  getc(ifile);
        if (c == EOF) return -1;
      }
	  }
    Size = readLong(ifile);
    if (Size < 0) return -1;
    /* make a buffer to hold the data and snag it from the input stream */
    str=(unsigned char *) malloc(Size);
    if (str == (unsigned char *) NULL)
      {
        printf("Memory allocation failed");
        return 0;
      }
    for (i=0; i<Size; i++)
    {
      c = getc(ifile);
      if (c == EOF) return -1;
      str[i] = c;
    }

    /* we currently skip thumbnails, since it does not make
     * any sense preserving them in a real world application
     */
    if (ID != THUMBNAIL_ID)
      {
        /* now finish up by formatting this binary data into
         * ASCII equivalent
         */
        if (strlen((const char *)PString) > 0)
	        fprintf(ofile, "8BIM#%d#%s=", ID, PString);
        else
	        fprintf(ofile, "8BIM#%d=", ID);
        if (ID == IPTC_ID)
          {
            formatString(ofile, "IPTC", 4);
            formatIPTCfromBuffer(ofile, (char *)str, Size);
          }
        else
          formatString(ofile, (char *)str, Size);
      }
    free(str);
    free(PString);

    resCount++;

    c = getc(ifile);
  }
  return resCount;
}

int tokenizer(unsigned inflag,char *token,int tokmax,char *line,
char *white,char *brkchar,char *quote,char eschar,char *brkused,
int *next,char *quoted);

char *super_fgets(char *b, int *blen, FILE *file)
{
  int
    c,
    len;

  unsigned char
    *q;

  len=*blen;
  for (q=(unsigned char *) b; ; q++)
  {
    c=fgetc(file);
    if (c == EOF || c == '\n')
      break;
    if ((q-(unsigned char *) b+1) >= (int) len)
      {
        int
          tlen;

        tlen=q-(unsigned char *) b;
        len<<=1;
        b=(char *) realloc((char *) b,(len+2));
        if (b == (char *) NULL)
          break;
        q=(unsigned char*)b+tlen;
      }
    *q=(unsigned char) c;
  }
  *blen=0;
  if (b != (char *) NULL)
    {
      int
        tlen;

      tlen=q-(unsigned char *)b;
      if (tlen == 0)
        return (char *) NULL;
      b[tlen] = '\0';
      *blen=++tlen;
    }
  return b;
}

#define BUFFER_SZ 4096

long parse8BIM(FILE *ifile, FILE *ofile)
{
  char
    brkused,
    quoted,
    *line,
    *token,
    *newstr,
    *name;

  int
    state,
    next;

  unsigned char
    dataset;

  unsigned int
    recnum;

  int
    inputlen = BUFFER_SZ;

  long
    savedolen = 0L,
    outputlen = 0L;

  fpos_t
    savedpos,
    currentpos;

  dataset = 0;
  recnum = 0;
  line = (char *) malloc(inputlen);     
  name = token = (char *)NULL;
  while((line = super_fgets(line,&inputlen,ifile))!=NULL)
  {
    state=0;
    next=0;

    token = (char *) malloc(inputlen);     
    newstr = (char *) malloc(inputlen);     
    while (tokenizer(0, token, inputlen, line, (char *) "", (char *) "=",
      (char *) "\"", 0, &brkused,&next,&quoted)==0)
    {
      if (state == 0)
        {                  
          int
            state,
            next;

          char
            brkused,
            quoted;

          state=0;
          next=0;
          while(tokenizer(0, newstr, inputlen, token, (char *) "",
            (char *) "#", (char *) "", 0, &brkused, &next, &quoted)==0)
          {
            switch (state)
            {
              case 0:
                if (strcmp(newstr,"8BIM")==0)
                  dataset = 255;
                else
                  dataset = (unsigned char) atoi(newstr);
                break;
              case 1:
                recnum = atoi(newstr);
                break;
              case 2:
                name = (char *) malloc(strlen(newstr)+1);
                if (name)
                  strcpy(name,newstr);  
                break;
            }
            state++;
          }
        }
      else
        if (state == 1)
          {
            int
              next;

            unsigned long
              len;

            char
              brkused,
              quoted;

            next=0;
            len = strlen(token);
            while (tokenizer(0, newstr, inputlen, token, (char *) "",
              (char *) "&", (char *) "", 0, &brkused, &next, &quoted)==0)
            {
              if (brkused && next > 0)
                {
                  char
                    *s = &token[next-1];

                  len -= convertHTMLcodes(s, strlen(s));
                }
            }

            if (dataset == 255)
              {
                unsigned char
                  nlen = 0;

                int
                  i;

                if (savedolen > 0)
                  {
                    long diff = outputlen - savedolen;
                    fgetpos(ofile, &currentpos);
                    fsetpos(ofile, &savedpos);
                    writeLong(ofile, diff);
                    fsetpos(ofile, &currentpos);
                    savedolen = 0L;
                  }
                if (outputlen & 1)
                  {
                    fputc(0x00, ofile);
                    outputlen++;
                  }
                fputs("8BIM", ofile);
                writeWord(ofile, recnum);
                outputlen += 6;
                if (name)
                  nlen = strlen(name);
                fputc(nlen, ofile);
                outputlen++;
                for (i=0; i<nlen; i++)
                  fputc(name[i], ofile);
                outputlen += nlen;
                if (!(nlen&1))
                  {
                    fputc(0, ofile);
                    outputlen++;
                  }
                if (recnum != IPTC_ID)
                  {
                    writeLong(ofile, len);
                    outputlen += 4;

                    next=0;
                    outputlen += len;
                    while (len--)
                      fputc(token[next++], ofile);

                    if (outputlen & 1)
                      {
                        fputc(0x00, ofile);
                        outputlen++;
                      }
                  }
                else
                  {
                    /* patch in a fake length for now and fix it later */
                    fgetpos(ofile, &savedpos);
                    writeLong(ofile,0xFFFFFFFFL);
                    outputlen += 4;
                    savedolen = outputlen;
                  }
              }
            else
              {
                if (len <= 0x7FFF)
                  {
                    fputc(0x1c, ofile);
                    fputc(dataset, ofile);
                    fputc(recnum & 255, ofile);
                    writeWord(ofile,len);
                    outputlen += 5;
                    next=0;
                    outputlen += len;
                    while (len--)
                      fputc(token[next++], ofile);
                  }
              }
          }
      state++;
    }
    free(token);
    token = (char *)NULL;
    free(newstr);
    newstr = (char *)NULL;
    free(name);
    name = (char *)NULL;
  }
  free(line);
  if (savedolen > 0)
    {
      long diff = outputlen - savedolen;
      fgetpos(ofile, &currentpos);
      fsetpos(ofile, &savedpos);
      writeLong(ofile, diff);
      fsetpos(ofile, &currentpos);
      savedolen = 0L;
    }
  return outputlen;
}

/* some defines for the different JPEG block types */
#define M_SOF0  0xC0            /* Start Of Frame N */
#define M_SOF1  0xC1            /* N indicates which compression process */
#define M_SOF2  0xC2            /* Only SOF0-SOF2 are now in common use */
#define M_SOF3  0xC3
#define M_SOF5  0xC5            /* NB: codes C4 and CC are NOT SOF markers */
#define M_SOF6  0xC6
#define M_SOF7  0xC7
#define M_SOF9  0xC9
#define M_SOF10 0xCA
#define M_SOF11 0xCB
#define M_SOF13 0xCD
#define M_SOF14 0xCE
#define M_SOF15 0xCF
#define M_SOI   0xD8
#define M_EOI   0xD9            /* End Of Image (end of datastream) */
#define M_SOS   0xDA            /* Start Of Scan (begins compressed data) */
#define M_APP0  0xe0
#define M_APP1  0xe1
#define M_APP2  0xe2
#define M_APP3  0xe3
#define M_APP4  0xe4
#define M_APP5  0xe5
#define M_APP6  0xe6
#define M_APP7  0xe7
#define M_APP8  0xe8
#define M_APP9  0xe9
#define M_APP10 0xea
#define M_APP11 0xeb
#define M_APP12 0xec
#define M_APP13 0xed
#define M_APP14 0xee
#define M_APP15 0xef

static int jpeg_get1(FILE *ifile, FILE *ofile)
{ 	
	int c;

	c = getc(ifile);
	if (c == EOF) return EOF;
  fputc(c, ofile);
	return c;
}

static int jpeg_skip1(FILE *ifile)
{ 	
	int c;

	c = getc(ifile);
	if (c == EOF) return EOF;
	return c;
}

static int jpeg_read_remaining(FILE *ifile, FILE *ofile)
{
 	int c;

  while ((c = jpeg_get1(ifile, ofile)) != EOF) continue;
	return M_EOI;
}

static int jpeg_skip_variable(FILE *ifile, FILE *ofile)
{ 
	unsigned int  length;
	int c1,c2;

  if ((c1 = jpeg_get1(ifile, ofile)) == EOF) return M_EOI;
  if ((c2 = jpeg_get1(ifile, ofile)) == EOF) return M_EOI;

	length = (((unsigned char) c1) << 8) + ((unsigned char) c2);
	length -= 2;

	while (length--)
		if (jpeg_get1(ifile, ofile) == EOF) return M_EOI;

	return 0;
}

static int jpeg_skip_variable2(FILE *ifile, FILE *ofile)
{ 
	unsigned int  length;
	int c1,c2;

  if ((c1 = getc(ifile)) == EOF) return M_EOI;
  if ((c2 = getc(ifile)) == EOF) return M_EOI;

	length = (((unsigned char) c1) << 8) + ((unsigned char) c2);
	length -= 2;

	while (length--)
		if (getc(ifile) == EOF) return M_EOI;

	return 0;
}

static int jpeg_nextmarker(FILE *ifile, FILE *ofile)
{
  int c;

  /* transfer anything until we hit 0xff */
  do {
    c = getc(ifile);
	  if (c == EOF)
      return M_EOI; /* we hit EOF */
	  else
	    if (c != 0xff)
        fputc(c, ofile);
  } while (c != 0xff);

  /* get marker byte, swallowing possible padding */
  do {
    c = getc(ifile);
	  if (c == EOF)
      return M_EOI; /* we hit EOF */
  } while (c == 0xff);

  return c;
}

static int jpeg_skip_till_marker(FILE *ifile, int marker)
{ 
  int c, i;

  do {
    /* skip anything until we hit 0xff */
    i = 0;
    do {
      c = getc(ifile);
      i++;
	    if (c == EOF)
        return M_EOI; /* we hit EOF */
    } while (c != 0xff);

    /* get marker byte, swallowing possible padding */
    do {
      c = getc(ifile);
	    if (c == EOF)
        return M_EOI; /* we hit EOF */
    } while (c == 0xff);
  } while (c != marker);
  return c;
}

static char psheader[] = "\xFF\xED\0\0Photoshop 3.0\08BIM\x04\x04\0\0\0\0";

/* Embed binary IPTC data into a JPEG image. */
int jpegembed(FILE *ifile, FILE *ofile, FILE *iptc)
{
	unsigned int marker;
	unsigned int done = 0;
	unsigned int len;
  int inx;
	struct stat sb;

	if (jpeg_get1(ifile, ofile) != 0xFF)
		return 0;
	if (jpeg_get1(ifile, ofile) != M_SOI)
		return 0;

	while (!done) {
		marker = jpeg_nextmarker(ifile, ofile);
		if (marker == M_EOI) { /* EOF */
			break;
		} else if (marker != M_APP13) {
			fputc(0xff, ofile);
      fputc(marker, ofile); 
		}

		switch (marker) {
			case M_APP13:
				/* we are going to write a new APP13 marker, so don't output the old one */
				jpeg_skip_variable2(ifile, ofile);
				break;

			case M_APP0:
				/* APP0 is in each and every JPEG, so when we hit APP0 we insert our new APP13! */
				jpeg_skip_variable(ifile, ofile);

        if (iptc != (FILE *)NULL) {
          fstat(fileno(iptc),&sb);
		      len = sb.st_size;
				  if (len & 1) len++; /* make the length even */
				  psheader[ 2 ] = (len+16)>>8;
				  psheader[ 3 ] = (len+16)&0xff;
				  for (inx = 0; inx < 18; inx++)
					  fputc(psheader[inx], ofile);
				  jpeg_read_remaining(iptc, ofile);
		      len = sb.st_size;
				  if (len & 1)
            fputc(0, ofile);
        }
				break;

			case M_SOS:								
				/* we hit data, no more marker-inserting can be done! */
				jpeg_read_remaining(ifile, ofile);
				done = 1;
				break;
			
			default:
				jpeg_skip_variable(ifile, ofile);
				break;
		}
	}
	return 1;
}

/* Extract any APP13 binary data into a file. */
int jpegextract(FILE *ifile, FILE *ofile)
{
	unsigned int marker;
	unsigned int done = 0;

	if (jpeg_skip1(ifile) != 0xFF)
		return 0;
	if (jpeg_skip1(ifile) != M_SOI)
		return 0;

	while (!done) {
		marker = jpeg_skip_till_marker(ifile, M_APP13);
		if (marker == M_APP13) {
      marker = jpeg_nextmarker(ifile, ofile);
			break;
		}
	}
	return 1;
}

int main(int argc, char *argv[])
{            
  int
    length;

  unsigned char
    *buffer;

  int
    i,
    align,
    mode; /* iptc binary, or iptc text */

  FILE
    *ifile = stdin,
    *ofile = stdout,
    *xfile;

  char
    c,
    *usage = (char *) "usage: iptcutil -t | -b | -x file [-i file] [-o file] <input >output";

  if( argc < 2 )
    {
      printf(usage);
	    return 1;
    }

  align = mode = 0;
  length = -1;
  xfile = (FILE *) NULL;
  buffer = (unsigned char *)NULL;

  for (i=1; i<argc; i++)
  {
    c = argv[i][0];
    if (c == '-' || c == '/')
      {
        c = argv[i][1];
        switch( c )
        {
        case 'a':
            /* align the binary output on a word boundary */
	        align = 1;
	        break;
        case 't':
	        mode = 1;
#ifdef _VISUALC_
          /* Set "stdout" to binary mode: */
          _setmode( _fileno( ofile ), _O_BINARY );
#endif
	        break;
        case 'b':
	        mode = 0;
#ifdef _VISUALC_
          /* Set "stdin" to binary mode: */
          _setmode( _fileno( ifile ), _O_BINARY );
#endif
	        break;
        case 'i':
          if (mode == 0 || mode == 2 || mode == 3)
            ifile = fopen(argv[++i], "rb");
          else
            ifile = fopen(argv[++i], "rt");
          if (ifile == (FILE *)NULL)
            {
	            printf("Unable to open: %s\n", argv[i]);
              return 1;
            }
	        break;
        case 'o':
          if (mode == 0)
            ofile = fopen(argv[++i], "wt");
          else
            ofile = fopen(argv[++i], "wb");
          if (ofile == (FILE *)NULL)
            {
	            printf("Unable to open: %s\n", argv[i]);
              return 1;
            }
	        break;
        case 's':
          mode = 3;
#ifdef _VISUALC_
          /* Set "stdout" to binary mode: */
          _setmode( _fileno( ofile ), _O_BINARY );
          /* Set "stdin" to binary mode: */
          _setmode( _fileno( ifile ), _O_BINARY );
#endif
	        break;
        case 'x':
          mode = 2;
#ifdef _VISUALC_
          /* Set "stdout" to binary mode: */
          _setmode( _fileno( ofile ), _O_BINARY );
          /* Set "stdin" to binary mode: */
          _setmode( _fileno( ifile ), _O_BINARY );
#endif
          xfile = fopen(argv[++i], "rb");
          if (xfile == (FILE *)NULL)
            {
	            printf("Unable to open: %s\n", argv[i]);
              return 1;
            }
	        break;
        default:
	        printf("Unknown option: %s\n", argv[i]);
	        return 1;
        }
      }
    else
      {
        printf(usage);
	      return 1;
      }
  }

  if (mode == 0) /* handle binary file info */
    {
      c = getc(ifile);
	    if (c == 0x1c)
        {
          ungetc(c, ifile);
          formatIPTC(ifile, ofile);
	      }
      ungetc(c,ifile);
      format8BIM(ifile, ofile);
    }
  else if (mode == 1) /* handle text form of iptc info */
    {
      long outputlen = parse8BIM(ifile, ofile);
      if (align && (outputlen & 1))
        fputc(0x00, ofile);
    }
  else if (mode == 2) /* handle writing iptc info into JPEG */
    jpegembed(ifile, ofile, xfile);
  else if (mode == 3) /* handle writing iptc info into JPEG */
    {
	    unsigned int marker;

		  marker = jpeg_skip_till_marker(ifile, M_SOI);
      if (marker == M_SOI)
      {
        fputc(0xFF, ofile);
        fputc(M_SOI, ofile);
		    jpeg_read_remaining(ifile, ofile);
      }
    }

  fclose( ifile );
  fclose( ofile );
  if (mode == 2)
    fclose( xfile );
  return 0;
}

/*
	This routine is a generalized, finite state token parser. It allows
    you extract tokens one at a time from a string of characters.  The
    characters used for white space, for break characters, and for quotes
    can be specified. Also, characters in the string can be preceded by
    a specifiable escape character which removes any special meaning the
    character may have.

	There are a lot of formal parameters in this subroutine call, but
	once you get familiar with them, this routine is fairly easy to use.
	"#define" macros can be used to generate simpler looking calls for
	commonly used applications of this routine.

	First, some terminology:

	token:		used here, a single unit of information in
				the form of a group of characters.

	white space:	space that gets ignored (except within quotes
				or when escaped), like blanks and tabs.  in
				addition, white space terminates a non-quoted
				token.

	break character: a character that separates non-quoted tokens.
				commas are a common break character.  the
				usage of break characters to signal the end
				of a token is the same as that of white space,
				except multiple break characters with nothing
				or only white space between generate a null
				token for each two break characters together.

				for example, if blank is set to be the white
				space and comma is set to be the break
				character, the line ...

				A, B, C ,  , DEF

				... consists of 5 tokens:

				1)	"A"
				2)	"B"
				3)	"C"
				4)	""      (the null string)
				5)	"DEF"

	quote character: 	a character that, when surrounding a group
				of other characters, causes the group of
				characters to be treated as a single token,
				no matter how many white spaces or break
				characters exist in the group.	also, a
				token always terminates after the closing
				quote.	for example, if ' is the quote
				character, blank is white space, and comma
				is the break character, the following
				string ...

				A, ' B, CD'EF GHI

				... consists of 4 tokens:

				1)	"A"
				2)	" B, CD" (note the blanks & comma)
				3)	"EF"
				4)	"GHI"

				the quote characters themselves do
				not appear in the resultant tokens.  the
				double quotes are delimiters i use here for
				documentation purposes only.

	escape character:	a character which itself is ignored but
				which causes the next character to be
				used as is.  ^ and \ are often used as
				escape characters.  an escape in the last
				position of the string gets treated as a
				"normal" (i.e., non-quote, non-white,
				non-break, and non-escape) character.
				for example, assume white space, break
				character, and quote are the same as in the
				above examples, and further, assume that
				^ is the escape character.  then, in the
				string ...

				ABC, ' DEF ^' GH' I ^ J K^ L ^

				... there are 7 tokens:

				1)	"ABC"
				2)	" DEF ' GH"
				3)	"I"
				4)	" "     (a lone blank)
				5)	"J"
				6)	"K L"
				7)	"^"     (passed as is at end of line)


	OK, now that you have this background, here's how to call "tokenizer":

	result=tokenizer(flag,token,maxtok,string,white,break,quote,escape,
		      brkused,next,quoted)

	result: 	0 if we haven't reached EOS (end of string), and
			1 if we have (this is an "int").

	flag:		right now, only the low order 3 bits are used.
			1 => convert non-quoted tokens to upper case
			2 => convert non-quoted tokens to lower case
			0 => do not convert non-quoted tokens
			(this is a "char").

	token:		a character string containing the returned next token
			(this is a "char[]").

	maxtok: 	the maximum size of "token".  characters beyond
			"maxtok" are truncated (this is an "int").

	string: 	the string to be parsed (this is a "char[]").

	white:		a string of the valid white spaces.  example:

			char whitesp[]={" \t"};

			blank and tab will be valid white space (this is
			a "char[]").

	break:		a string of the valid break characters.  example:

			char breakch[]={";,"};

			semicolon and comma will be valid break characters
			(this is a "char[]").

			IMPORTANT:  do not use the name "break" as a C
			variable, as this is a reserved word in C.

	quote:		a string of the valid quote characters.  an example
			would be

			char whitesp[]={"'\"");

			(this causes single and double quotes to be valid)
			note that a token starting with one of these characters
			needs the same quote character to terminate it.

			for example,

			"ABC '

			is unterminated, but

			"DEF" and 'GHI'

			are properly terminated.  note that different quote
			characters can appear on the same line; only for
			a given token do the quote characters have to be
			the same (this is a "char[]").

	escape: 	the escape character (NOT a string ... only one
			allowed).  use zero if none is desired (this is
			a "char").

	brkused:	the break character used to terminate the current
			token.	if the token was quoted, this will be the
			quote used.  if the token is the last one on the
			line, this will be zero (this is a pointer to a
			"char").

	next:		this variable points to the first character of the
			next token.  it gets reset by "tokenizer" as it steps
			through the string.  set it to 0 upon initialization,
			and leave it alone after that.	you can change it
			if you want to jump around in the string or re-parse
			from the beginning, but be careful (this is a
			pointer to an "int").

	quoted: 	set to 1 (true) if the token was quoted and 0 (false)
			if not.  you may need this information (for example:
			in C, a string with quotes around it is a character
			string, while one without is an identifier).

			(this is a pointer to a "char").
*/

/* states */

#define IN_WHITE 0
#define IN_TOKEN 1
#define IN_QUOTE 2
#define IN_OZONE 3

int _p_state;	   /* current state	 */
unsigned _p_flag;  /* option flag	 */
char _p_curquote;  /* current quote char */
int _p_tokpos;	   /* current token pos  */

/* routine to find character in string ... used only by "tokenizer" */

int sindex(char ch,char *string)
{
  char *cp;
  for(cp=string;*cp;++cp)
    if(ch==*cp)
      return (int)(cp-string);	/* return postion of character */
  return -1;			/* eol ... no match found */
}

/* routine to store a character in a string ... used only by "tokenizer" */

void chstore(char *string,int max,char ch)
{
  char c;
  if(_p_tokpos>=0&&_p_tokpos<max-1)
  {
    if(_p_state==IN_QUOTE)
      c=ch;
    else
      switch(_p_flag&3)
      {
	    case 1: 	    /* convert to upper */
	      c=toupper(ch);
	      break;

	    case 2: 	    /* convert to lower */
	      c=tolower(ch);
	      break;

	    default:	    /* use as is */
	      c=ch;
	      break;
      }
    string[_p_tokpos++]=c;
  }
  return;
}

int tokenizer(unsigned inflag,char *token,int tokmax,char *line,
  char *white,char *brkchar,char *quote,char eschar,char *brkused,
    int *next,char *quoted)
{
  int qp;
  char c,nc;

  *brkused=0;		/* initialize to null */
  *quoted=0;		/* assume not quoted  */

  if(!line[*next])	/* if we're at end of line, indicate such */
    return 1;

  _p_state=IN_WHITE;   /* initialize state */
  _p_curquote=0;	   /* initialize previous quote char */
  _p_flag=inflag;	   /* set option flag */

  for(_p_tokpos=0;(c=line[*next]);++(*next))	/* main loop */
  {
    if((qp=sindex(c,brkchar))>=0)  /* break */
    {
      switch(_p_state)
      {
	    case IN_WHITE:		/* these are the same here ...	*/
	    case IN_TOKEN:		/* ... just get out		*/
	    case IN_OZONE:		/* ditto			*/
	      ++(*next);
	      *brkused=brkchar[qp];
	      goto byebye;

	    case IN_QUOTE:		 /* just keep going */
	      chstore(token,tokmax,c);
	      break;
      }
    }
    else if((qp=sindex(c,quote))>=0)  /* quote */
    {
      switch(_p_state)
      {
	    case IN_WHITE:	 /* these are identical, */
	      _p_state=IN_QUOTE; /* change states   */
	      _p_curquote=quote[qp]; /* save quote char */
	      *quoted=1;	/* set to true as long as something is in quotes */
	      break;

	    case IN_QUOTE:
	      if(quote[qp]==_p_curquote) /* same as the beginning quote? */
	      {
	        _p_state=IN_OZONE;
	        _p_curquote=0;
	      }
	      else
	        chstore(token,tokmax,c); /* treat as regular char */
	      break;

	    case IN_TOKEN:
	    case IN_OZONE:
	      *brkused=c; /* uses quote as break char */
	      goto byebye;
      }
    }
    else if((qp=sindex(c,white))>=0) /* white */
    {
      switch(_p_state)
      {
	    case IN_WHITE:
	    case IN_OZONE:
	      break;		/* keep going */

	    case IN_TOKEN:
	      _p_state=IN_OZONE;
	      break;

	    case IN_QUOTE:
	      chstore(token,tokmax,c); /* it's valid here */
	      break;
      }
    }
    else if(c==eschar)  /* escape */
    {
      nc=line[(*next)+1];
      if(nc==0) 		/* end of line */
      {
	    *brkused=0;
	    chstore(token,tokmax,c);
	    ++(*next);
	    goto byebye;
      }
      switch(_p_state)
      {
	    case IN_WHITE:
	      --(*next);
	      _p_state=IN_TOKEN;
	      break;

	    case IN_TOKEN:
	    case IN_QUOTE:
	      ++(*next);
	      chstore(token,tokmax,nc);
	      break;

	    case IN_OZONE:
	      goto byebye;
      }
    }
    else	/* anything else is just a real character */
    {
      switch(_p_state)
      {
	    case IN_WHITE:
	      _p_state=IN_TOKEN; /* switch states */

	    case IN_TOKEN:		 /* these 2 are     */
	    case IN_QUOTE:		 /*  identical here */
	      chstore(token,tokmax,c);
	      break;

	    case IN_OZONE:
	      goto byebye;
      }
    }
  }		/* end of main loop */

byebye:
  token[_p_tokpos]=0;	/* make sure token ends with EOS */

  return 0;
}
