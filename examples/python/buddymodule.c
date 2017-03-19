#include <Python.h>
#include <structmember.h>

typedef struct {
    PyObject_HEAD
    PyObject *firstname;
    PyObject *lastname;
} BuddyObject;

static int
buddy_init(BuddyObject *self, PyObject *args, PyObject *kwds)
{
    PyObject *firstname = Py_None;
    PyObject *lastname = Py_None;
    PyObject *tmp = NULL;

    static char *kwlist[] = {"firstname", "lastname", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OO", kwlist,
                     &firstname, &lastname))
        return -1;

    tmp = self->firstname;
    if (firstname == Py_None) {
        self->firstname = PyString_FromString("");
    }
    else {
        Py_INCREF(firstname);
        self->firstname = firstname;
    }
    Py_XDECREF(tmp);

    tmp = self->lastname;
    if (lastname == Py_None) {
        self->lastname = PyString_FromString("");
    }
    else {
        Py_INCREF(lastname);
        self->lastname = lastname;
    }
    Py_XDECREF(tmp);

    return 0;
}


static PyObject *
Buddy_name(BuddyObject *self, PyObject *unused)
{
    PyObject *args = NULL;
    PyObject *format = NULL;
    PyObject *result = NULL;

    format = PyString_FromString("%s %s");
    if (format == NULL)
        goto fail;

    if (self->firstname == NULL) {
        PyErr_SetString(PyExc_AttributeError, "firstname");
        goto fail;
    }

    if (self->lastname == NULL) {
        PyErr_SetString(PyExc_AttributeError, "lastname");
        goto fail;
    }

    args = Py_BuildValue("OO", self->firstname, self->lastname);
    if (args == NULL)
        goto fail;

    result = PyString_Format(format, args);

fail:
    Py_XDECREF(format);
    Py_XDECREF(args);

    return result;
}

static PyObject *
Buddy_init(BuddyObject *self, PyObject *unused)
{
    PyObject *args = NULL;
    PyObject *format = NULL;
    PyObject *result = NULL;

    format = PyString_FromString("%s %s");
    if (format == NULL)
        goto fail;

    if (self->firstname == NULL) {
        PyErr_SetString(PyExc_AttributeError, "firstname");
        goto fail;
    }

    if (self->lastname == NULL) {
        PyErr_SetString(PyExc_AttributeError, "lastname");
        goto fail;
    }

    args = Py_BuildValue("OO", self->firstname, self->lastname);
    if (args == NULL)
        goto fail;

    result = PyString_Format(format, args);

fail:
    Py_XDECREF(format);
    Py_XDECREF(args);

    return result;
}

static PyMemberDef Buddy_members[] = {
    {"firstname", T_OBJECT_EX, offsetof(BuddyObject, firstname), 0,
     "first name"},
    {"lastname", T_OBJECT_EX, offsetof(BuddyObject, lastname), 0,
     "last name"},
    {NULL}  /* Sentinel */
};

static PyMethodDef Buddy_methods[] = {
    {"name", (PyCFunction)Buddy_name, METH_NOARGS,
     "Return the name, combining the firstname and lastname."
    },
    {"init", (PyCFunction)Buddy_init, METH_NOARGS,
     "Initialize the Buddy hardware device."
    },
    {NULL}
};

static PyTypeObject BuddyObjectType = {
    PyObject_HEAD_INIT(NULL)
    0,                              /* ob_size        */
    "buddy.Buddy",                  /* tp_name        */
    sizeof(BuddyObject),            /* tp_basicsize   */
    0,                              /* tp_itemsize    */
    0,                              /* tp_dealloc     */
    0,                              /* tp_print       */
    0,                              /* tp_getattr     */
    0,                              /* tp_setattr     */
    0,                              /* tp_compare     */
    0,                              /* tp_repr        */
    0,                              /* tp_as_number   */
    0,                              /* tp_as_sequence */
    0,                              /* tp_as_mapping  */
    0,                              /* tp_hash        */
    0,                              /* tp_call        */
    0,                              /* tp_str         */
    0,                              /* tp_getattro    */
    0,                              /* tp_setattro    */
    0,                              /* tp_as_buffer   */
    Py_TPFLAGS_DEFAULT,             /* tp_flags       */
    "Buddy objects are buddy.",     /* tp_doc         */
    0,                              /* tp_traverse       */
    0,                              /* tp_clear          */
    0,                              /* tp_richcompare    */
    0,                              /* tp_weaklistoffset */
    0,                              /* tp_iter           */
    0,                              /* tp_iternext       */
    Buddy_methods,                  /* tp_methods        */
    Buddy_members,                  /* tp_members        */
    0,                              /* tp_getset         */
    0,                              /* tp_base           */
    0,                              /* tp_dict           */
    0,                              /* tp_descr_get      */
    0,                              /* tp_descr_set      */
    0,                              /* tp_dictoffset     */
    (initproc)buddy_init,           /* tp_init           */
};

PyMODINIT_FUNC
initbuddy(void)
{
    PyObject *m;

    BuddyObjectType.tp_new = PyType_GenericNew;

    if (PyType_Ready(&BuddyObjectType) < 0)
        return;

    m = Py_InitModule3("buddy", NULL,
        "Example module that creates an extension type.");

    if (m == NULL)
        return;

    Py_INCREF(&BuddyObjectType);
    PyModule_AddObject(m, "Buddy", 
                       (PyObject *) &BuddyObjectType);
}