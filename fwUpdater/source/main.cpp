/* -------------------------------------------------------------------------- */
/* FILENAME:
   AUTHOR:      Daniele Aimo (d.aimo@arduino.cc / maidnl74@gmail.com)
   DATE:        20230801
   REVISION:    0.0.1
   DESCRIPTION:
   LICENSE:     Copyright (c) 2024 Arduino SA
                This Source Code Form is subject to the terms fo the Mozilla
                Public License (MPL), v 2.0. You can obtain a copy of the MPL
                at http://mozilla.org/MPL/2.0/.
   NOTES:                                                                     */
/* -------------------------------------------------------------------------- */
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
using namespace std;

#define EXP_TYPE_DIGITAL 1
#define EXP_TYPE_ANALOG 2
#define EXP_TYPE_CELLULAR 3

int getUserNumber(bool &good) {
  string usr_string;

  cin >> usr_string;
  bool all_digits = true;
  for (int i = 0; i < usr_string.size(); i++) {
    if (!isdigit(static_cast<unsigned char>(usr_string[i]))) {
      all_digits = false;
      cout << ">> ERROR: Unrecognized input! (not a number)" << endl;
      break;
    }
  }

  if (!all_digits) {
    good = false;
    return -1;
  } else {
    good = true;
    return atoi(usr_string.c_str());
  }
}

int print_single_char_on_file(FILE *fp, unsigned char ch, bool comma = true) {
  int rv = 0;
  if (fp != nullptr) {
    rv = 4;
    fprintf(fp, "0x%02X", ch);
    if (comma) {
      fprintf(fp, ", ");
      rv++;
    }
  }
  return rv;
}

static const char fileOutDefault[] = "fwUpdate.h";
static const char fileOutAnalog[] = "fwUpdateAnalog.h";
static const char fileOutDigital[] = "fwUpdateDigital.h";
static const char fileOutCellular[] = "fwUpdateCellular.h";

static const char nameDefault[] ="UNKNOWN";
static const char nameAnalog[] = "ANALOG";
static const char nameDigital[] = "DIGITAL";
static const char nameCellular[] = "CELLULAR";

static const unsigned char OA_header[] =
    "const unsigned char opta_analog_fw_update[] = {";
static const unsigned char OD_header[] =
    "const unsigned char opta_digital_fw_update[] = {";
static const unsigned char OC_header[] =
    "const unsigned char opta_cellular_fw_update[] = {";    

/* __________________________________________________________________________ */
bool write_update(const char *filename, unsigned char e_type, unsigned char M,
                  unsigned char m, unsigned char r, unsigned char *fw,
                  uint32_t fw_size) {
  bool rv = true;
  FILE *fp = fopen(filename, "w");
  int printed = 0;

  if (fp == nullptr) {
    cout << "ERROR Unable to open output file!" << endl;
    rv = false;
    goto EXIT_WRITE_UPDATE_AND_CLOSE;
  }

  if (e_type == EXP_TYPE_ANALOG) {
    /* print declaration of array containing the FW update */
    fprintf(fp, "%s\n", OA_header);
  } else if (e_type == EXP_TYPE_DIGITAL) {
    fprintf(fp, "%s\n", OD_header);
  } else if (e_type == EXP_TYPE_CELLULAR) {
    fprintf(fp, "%s\n", OC_header);
  }else {
    cout << "ERROR Unrecognized option" << endl;
    rv = false;
    goto EXIT_WRITE_UPDATE;
  }

  if (fw == nullptr) {
    cout << "ERROR FW is missing" << endl;
    rv = false;
    goto EXIT_WRITE_UPDATE;
  }

  for (int i = 0; i < fw_size; i++) {
    if (i == fw_size - 1) {
      printed += print_single_char_on_file(fp, fw[i], false);
    } else {
      printed += print_single_char_on_file(fp, fw[i]);
    }
    if (printed > 0 && (printed % 80) == 0) {
      fprintf(fp, "\n");
    }
  }
  /* the fw is finished, put a newline */
  fprintf(fp, ",\n");

  print_single_char_on_file(fp, e_type);
  print_single_char_on_file(fp, M);
  print_single_char_on_file(fp, m);
  print_single_char_on_file(fp, r, false);
  fprintf(fp, " };\n");
EXIT_WRITE_UPDATE_AND_CLOSE:
  fclose(fp);
EXIT_WRITE_UPDATE:
  return rv;
}

/* __________________________________________________________________________ */
bool print_expansion_selection(int sel) {
  bool rv = true;
  if (sel == EXP_TYPE_ANALOG) {
    cout << "\n--> *** ANALOG *** expansion type selected" << endl;  
  } else if (sel == EXP_TYPE_DIGITAL) {
    cout << "\n--> *** DIGITAL *** expansion type selected" << endl;
  } else if (sel == EXP_TYPE_CELLULAR) {
    cout << "\n--> *** CELLULAR *** expansion type selected" << endl;    
  } 
  else {
    cout << ">> ERROR unrecognized option!" << endl;
    rv = false;
  }
  return rv;
}

/* __________________________________________________________________________ */
int select_expansion(bool &good) {
  cout << "Please select the type of expansion:" << endl;
  cout << " 1. OPTA DIGITAL " << endl;
  cout << " 2. OPTA ANALOG " << endl;
  cout << " 3. OPTA CELLULAR " << endl;
  cout << "Or type 0 to exit " << endl;
  cout << ":> ";
  int rv = getUserNumber(good);
  return rv; 
}

/* __________________________________________________________________________ */
unsigned char get_version_digit(const char *type) {
  bool good = false;
  int rv = 0;

  while(!good) {
    cout << "Please enter the ";
    cout << type;
    cout << " number of the FW release: " << endl;
    cout << ":> ";
    rv = (unsigned char)getUserNumber(good);
    if(!good) {
      cout << "\n -> malformed expression" << endl;
    }
  }
  return rv;
}

/* __________________________________________________________________________ */
void get_fw_filename(string &filename, uint32_t &fsize) {

  while(1) {
    cout << "Please Enter the path where FW (bin format) is located" << endl;
    cout << ":> ";
    cin >> filename;

    struct stat info;
    if (stat(filename.c_str(), &info) != 0) {
      cout << "ERROR Unable to stat file! (Did you enter the correct path?)"
           << endl;
    }
    else {
      fsize = info.st_size;
      break;
    }
  }
}

/* __________________________________________________________________________ */
unsigned char *read_fw_filename(string &filename, uint32_t fsize) {
  FILE *fp = fopen(filename.c_str(), "r+b");
  if (fp == nullptr) {
    cout << "ERROR Unable to open the file " << filename << endl;
    exit(1);
  }

  unsigned char *rv = new unsigned char[fsize];
  if (rv == NULL) {
    cout << "ERROR Unable to allocate memory... Exit" << endl;
    exit(1);
  }
  size_t blocks_read = fread(rv, fsize, 1, fp);
  if (blocks_read != 1) {
    cout << "ERROR! Unable to read all the file" << endl;
    exit(2);
  }
  fclose(fp);
  return rv;
}

/* -------------------------------------------------------------------------- */
int main(int argc, char *argv[]) {
  int exp_type = 0;
  unsigned char fw_version_major = 0;
  unsigned char fw_version_minor = 0;
  unsigned char fw_version_release = 0;
  
  char *outfilename = (char *)fileOutDefault;
  char *expname = (char *)nameDefault;
  
  string filename;
  uint32_t fsize = 0;
  
  unsigned char *fcontent = nullptr;
  

  cout << "*** Opta Fw Update Maker ***" << endl;
  cout << endl;
  cout << "This program packs an Expansions fw into .h files" << endl;
  cout << endl;

  while (1) {

    bool good = false;
    exp_type = select_expansion(good);
    if(!good) {
      continue;
    }

    if(exp_type == 0) {
      break;
    }

    if(!print_expansion_selection(exp_type)) {
      continue;
    }
    
    fw_version_major = get_version_digit("MAJOR");
    fw_version_minor = get_version_digit("MINOR");
    fw_version_release = get_version_digit("RELEASE");

    cout << "*** You entered the FW version: " << (int)fw_version_major << "."
         << (int)fw_version_minor << "." << (int)fw_version_release << endl;

    fsize = 0;
    get_fw_filename(filename,fsize);
    if(fcontent != nullptr) {
      delete []fcontent;
      fcontent = nullptr;
    }
    fcontent = read_fw_filename(filename, fsize);

    if (exp_type == EXP_TYPE_ANALOG) {
      outfilename = (char *)fileOutAnalog;
      expname = (char *)nameAnalog;
    } else if (exp_type == EXP_TYPE_DIGITAL) {
      outfilename = (char *)fileOutDigital;
      expname = (char *)nameDigital;
    } else if (exp_type == EXP_TYPE_CELLULAR) {
      outfilename = (char *)fileOutCellular;
      expname = (char *)nameCellular;
    } 
    
    if (!write_update(outfilename, exp_type, fw_version_major,
                      fw_version_minor, fw_version_release, fcontent,
                      fsize)) {
      cout << "ERROR Unable to write update on file " << outfilename << endl;
      exit(3);
    }
    else {
      cout << expname << " EXPANSION fw update saved on file:" << endl;
      cout << "  ---> " << outfilename << endl;
      cout << "  FW size: " << fsize << endl << endl;
    }
  }
}
