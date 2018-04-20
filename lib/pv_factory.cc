// -*- c++ -*-
//
// pv_factory.cc
//
// kasemir@lanl.gov

#include<stdio.h>
#include<stdlib.h>
#include "pvBindings.h"

static int edmReadOnly = 0;
static pvBindingClass pvObj;

void setReadOnly ( void )
{
  edmReadOnly = 1;
}

void setReadWrite ( void )
{
  edmReadOnly = 0;
}

int isReadOnly ( void )
{
  return edmReadOnly;
}

int pend_io ( double sec )
{

  return (int) pvObj.pend_io( sec );

}

int pend_event ( double sec )
{

  return (int) pvObj.pend_event( sec );

}

void task_exit ( void ) {

  pvObj.task_exit();

}

// Available PV_Factories:
       PV_Factory *the_PV_Factory   = new PV_Factory();

//extern "C" static void remove_pv_factories()
static void remove_pv_factories()
{
    if (the_PV_Factory)
    {
        delete the_PV_Factory;
        the_PV_Factory = 0;
    }
}

PV_Factory::PV_Factory()
{
    strcpy( default_pv_type, "" );
    atexit(remove_pv_factories);
}

PV_Factory::~PV_Factory()
{

}

int PV_Factory::legal_pv_type (
  const char *pv_type )
{

char *supportedType;

  supportedType = pvObj.firstPvName();
  while ( supportedType ) {

    if ( strcmp( pv_type, supportedType ) == 0 ) {
      return 1;
    }

    supportedType = pvObj.nextPvName();

  }

  return 0;

}

void PV_Factory::set_default_pv_type (
  const char *pv_type )
{

  if ( legal_pv_type( pv_type ) ) {

    strncpy( default_pv_type, pv_type, 31 );
    default_pv_type[31] = 0;

  }
  else {

    fprintf(stderr,
     "Cannot set default PV type to '%s', using system default\n", pv_type );
    strcpy( default_pv_type, "" );

  }

}

void PV_Factory::clear_default_pv_type ( void )
{

  //fprintf( stderr, "clearing default pv type\n" );

  strcpy( default_pv_type, "" );

}


/* Parses a PV name into two components, class and PV. */
void PV_Factory::parse_pv_name(
    const char *full_pv_name, char *pv_class, char *pv_name)
{
    /****** SJS modification 05_12_12 for RHEL6 - replace ******
    char *slash = strchr(full_pv_name, '\\');
    ******* by ******/
    const char *slash = strchr(full_pv_name, '\\');
    /****** End of SJS modification *****/
    if (slash)
    {
        /* Compound name: split at first \ character. */
        snprintf(pv_class, MAX_PV_NAME, "%.*s",
            slash - full_pv_name, full_pv_name);
        snprintf(pv_name, MAX_PV_NAME, "%s", slash + 1);
    }
    else if (default_pv_type[0] != '\0')
    {
        /* If default class available use that. */
        snprintf(pv_class, MAX_PV_NAME, "%s", default_pv_type);
        snprintf(pv_name, MAX_PV_NAME, "%s", full_pv_name);
    }
    else
    {
        /* If no default class then use first class in pvObj. */
        snprintf(pv_class, MAX_PV_NAME, "%s", pvObj.firstPvName());
        snprintf(pv_name, MAX_PV_NAME, "%s", full_pv_name);
    }
}

class ProcessVariable *PV_Factory::create(const char *full_pv_name)
{
    char pv_class[MAX_PV_NAME];
    char pv_name[MAX_PV_NAME];
    parse_pv_name(full_pv_name, pv_class, pv_name);
    return pvObj.createNew(pv_class, pv_name);
}

/* Creates PV connection with given fixed size. */
class ProcessVariable *PV_Factory::create_size(
    const char *full_pv_name, size_t size)
{
    char pv_class[MAX_PV_NAME];
    char pv_name[MAX_PV_NAME];
    parse_pv_name(full_pv_name, pv_class, pv_name);
    return pvObj.createNew_size(pv_class, pv_name, size);
}


// These two should be static, but then "friend" doesn't work,
// so the CallBackInfo would have to be public which is
// not what I want, either...
size_t hash(const PVCallbackInfo *item, size_t N)
{   return ((size_t)item->func*41 + (size_t)item->userarg*43)%N; }

bool equals(const PVCallbackInfo *lhs,
            const PVCallbackInfo *rhs)
{
    return lhs->func == rhs->func &&
        lhs->userarg == rhs->userarg;
}

size_t hash(const NodeNameInfo *item, size_t N)
{   return generic_string_hash(item->nodeName, N); }

bool equals(const NodeNameInfo *lhs,
            const NodeNameInfo *rhs)
{   return strcmp(lhs->nodeName, rhs->nodeName) == 0; }

ProcessVariable::ProcessVariable(const char *_name)
{
    name = strdup(_name);
    refcount = 1;
    numTimesConnected = numTimesDisconnected = numValueChangeEvents = 0;
    nodeName = NULL;
}

ProcessVariable::~ProcessVariable()
{

    //fprintf( stderr, "~ProcessVariable, name=[%s]\n", name );
    //fprintf( stderr, "num times connected = %-d, disconnected = %-d\n",
    // numTimesConnected, numTimesDisconnected );
    //fprintf( stderr, "num value change events = %-d\n", numValueChangeEvents );
    //if ( nodeName ) {
    //  fprintf( stderr, "node name = [%s] [%-x]\n", nodeName, (int) nodeName );
    //}

    if (refcount != 0)
        fprintf( stderr,"ProcessVariable %s deleted with refcount %d\n",
               name, refcount);
    if ( name ) {
      free(name);
      name = NULL;
    }

}

// Some defaults for member functions:
int ProcessVariable::get_int() const
{   return (int) get_double(); }

size_t ProcessVariable::get_string(char *strbuf, size_t buflen) const
{
    sprintf(strbuf, "%g", get_double());
    return strlen(strbuf);
}

size_t ProcessVariable::get_enum_count() const
{   return 0; }

const char *ProcessVariable::get_enum(size_t i) const
{   return 0; }

const char *ProcessVariable::get_units() const
{   return ""; }

void ProcessVariable::add_conn_state_callback(PVCallback func, void *userarg)
{
    PVCallbackInfo *info = new PVCallbackInfo;
    info->func = func;
    info->userarg = userarg;
    // TODO: search for existing one?
    conn_state_callbacks.insert(info);
    // Perform initial callback in case we already have a value
    // (otherwise user would have to wait until the next change)
    if (is_valid()) {
        (*func)(this, userarg);
    }
}

void ProcessVariable::add_access_security_callback (
  PVCallback func, void *userarg
) {

PVCallbackInfo *info = new PVCallbackInfo;

  info->func = func;
  info->userarg = userarg;
  // TODO: search for existing one?
  access_security_callbacks.insert(info);
  // Perform initial callback in case we already have information
  // (otherwise user would have to wait until the next change)
  if (is_valid()) {
    (*func)(this, userarg);
  }

}

void ProcessVariable::set_node_name( const char *_nodeName ) {

  char *theName;
  NodeNameInfo info;
  info.nodeName = (char *) _nodeName;
  NodeNameInfoHash::iterator entry = nodeNames.find(&info);
  if (entry == nodeNames.end()) {
    NodeNameInfo *pinfo = new NodeNameInfo;
    pinfo->nodeName = strdup( (char *) _nodeName );
    nodeNames.insert(pinfo);
    theName = pinfo->nodeName;
  }
  else {
    theName = (*entry)->nodeName;
  }

  this->nodeName = theName;

}

void ProcessVariable::remove_conn_state_callback(PVCallback func, void *userarg)
{
    PVCallbackInfo info;
    info.func = func;
    info.userarg = userarg;
    PVCallbackInfoHash::iterator entry = conn_state_callbacks.find(&info);
    if (entry != conn_state_callbacks.end()) {
        PVCallbackInfo *p_item = *entry;
        conn_state_callbacks.erase(entry);
        delete p_item;
    }
}

void ProcessVariable::remove_access_security_callback (
  PVCallback func, void *userarg
) {

PVCallbackInfo info;

  info.func = func;
  info.userarg = userarg;
  PVCallbackInfoHash::iterator entry = access_security_callbacks.find(&info);
  if (entry != access_security_callbacks.end()) {
    PVCallbackInfo *p_item = *entry;
    access_security_callbacks.erase(entry);
    delete p_item;
  }

}

void ProcessVariable::add_value_callback(PVCallback func, void *userarg)
{
    PVCallbackInfo *info = new PVCallbackInfo;
    info->func = func;
    info->userarg = userarg;
    // TODO: search for existing one?
    value_callbacks.insert(info);

    // Perform initial callback in case we already have a value
    // (otherwise user would have to wait until the next change)
    if (is_valid()) {
        recalc();
        (*func)(this, userarg);
    }
}

void ProcessVariable::remove_value_callback(PVCallback func, void *userarg)
{
    PVCallbackInfo info;
    info.func = func;
    info.userarg = userarg;
    PVCallbackInfoHash::iterator entry = value_callbacks.find(&info);
    if (entry != value_callbacks.end()) {
        PVCallbackInfo *p_item = *entry;
        value_callbacks.erase(entry);
        delete p_item;
    }
}

void ProcessVariable::do_conn_state_callbacks()
{
    PVCallbackInfo *info;

    if ( is_valid() ) {
      numTimesConnected++;
    }
    else {
      numTimesDisconnected++;
    }

    for (PVCallbackInfoHash::iterator entry = conn_state_callbacks.begin();
         entry != conn_state_callbacks.end();
         ++entry)
    {
        info = *entry;
        if (info->func)
            (*info->func) (this, info->userarg);
    }

}

int ProcessVariable::get_num_conn_state_callbacks ( void ) {

    PVCallbackInfo *info;
    int n = 0;

    for (PVCallbackInfoHash::iterator entry = conn_state_callbacks.begin();
         entry != conn_state_callbacks.end();
         ++entry)
    {
        info = *entry;
        if (info->func)
            n++;
    }

    return n;

}

void ProcessVariable::do_access_security_callbacks ( void ) {

PVCallbackInfo *info;

  for (PVCallbackInfoHash::iterator entry = access_security_callbacks.begin();
   entry != access_security_callbacks.end(); ++entry) {
    info = *entry;
    if (info->func) {
      (*info->func) (this, info->userarg);
    }
  }

}

void ProcessVariable::do_value_callbacks()
{
    PVCallbackInfo *info;

    numValueChangeEvents++;

    for (PVCallbackInfoHash::iterator entry = value_callbacks.begin();
         entry != value_callbacks.end();
         ++entry)
    {
        info = *entry;
        if (info->func) {
            (*info->func) (this, info->userarg);
	}
    }

}

int ProcessVariable::get_num_value_callbacks ( void ) {

    PVCallbackInfo *info;
    int n = 0;

    for (PVCallbackInfoHash::iterator entry = value_callbacks.begin();
         entry != value_callbacks.end();
         ++entry)
    {
        info = *entry;
        if (info->func) {
	  n++;
	}
    }

    return n;

}

void ProcessVariable::recalc() {
}

bool ProcessVariable::have_read_access() const
{
    return true;
}

bool ProcessVariable::have_write_access() const
{
    return true;
}

bool ProcessVariable::put(double value) {
  return true;
}

bool ProcessVariable::put(const char *dsp, double value) {
  return put(value);
}

bool ProcessVariable::put(const char *value) {
  return true;
}

bool ProcessVariable::put(const char *dsp, const char *value) {
  return put(value);
}

bool ProcessVariable::put(int value)
{   return true; }

bool ProcessVariable::put(const char *dsp, int value)
{   return put(value); }

bool ProcessVariable::putText(char *value) {
  return true;
}

bool ProcessVariable::putText(const char *dsp, char *value) {
  return putText(value);
}

bool ProcessVariable::putAck(short value)
{    return true; }

bool ProcessVariable::putAck(const char *dsp, short value)
{    return putAck(value); }

