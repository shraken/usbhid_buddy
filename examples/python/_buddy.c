#include <Python.h>
#include <usbhid_buddy.h>

hid_device* device;

/* Docstrings */
static char module_docstring[] =
    "This module provides an interface to the USBHID DAQ Buddy device using C.";
static char py_usbhid_buddy_init_docstring[] =
    "Initialization called on first boot of application.";
static char py_usbhid_write_packet_docstring[] =
    "Use the hidapi library to write a USBHID packet";
static char py_usbhid_read_packet_docstring[] =
    "Use the hidapi library to read a USBHID packet";
static char py_usbhid_buddy_cleanup_docstring[] =
    "Use the hidapi library to read a USBHID packet";

/* Available functions */
static PyObject *py_usbhid_buddy_init(PyObject *self, PyObject *args);
static PyObject *py_usbhid_write_packet(PyObject *self, PyObject *args);
static PyObject *py_usbhid_read_packet(PyObject *self, PyObject *args);
static PyObject *py_usbhid_buddy_cleanup(PyObject *self, PyObject *args);

/* Module specification */
static PyMethodDef module_methods[] = {
    {"usbhid_buddy_init", py_usbhid_buddy_init, METH_VARARGS, py_usbhid_buddy_init_docstring},
    {"usbhid_write_packet", py_usbhid_write_packet, METH_VARARGS, py_usbhid_buddy_init_docstring},
    {"usbhid_read_packet", py_usbhid_read_packet, METH_VARARGS, py_usbhid_read_packet_docstring},
    {"usbhid_buddy_cleanup", py_usbhid_buddy_cleanup, METH_VARARGS, py_usbhid_buddy_cleanup_docstring},
    {NULL, NULL, 0, NULL}
};

/* Initialize the module */
PyMODINIT_FUNC init_buddy(void)
{
    PyObject *m;

    m = Py_InitModule3("_buddy", module_methods, module_docstring);
    if (m == NULL)
        return;

    /* add all usbhid constants */
    /*
    if (PyModule_AddIntConstant(m, "USBHID_OUT_DATA_ID", USBHID_OUT_DATA_ID) != 0)
        return;
    if (PyModule_AddIntConstant(m, "USBHID_IN_DATA_ID", USBHID_IN_DATA_ID) != 0)
        return;

    if (PyModule_AddIntConstant(m, "USB_OUT_DAC_INDEX", USB_OUT_DAC_INDEX) != 0)
        return;
    */

    /* USB_APP_CODE enum */
    /*
    if (PyModule_AddIntConstant(m, "APP_CODE_CTRL", APP_CODE_CTRL) != 0)
        return;
    if (PyModule_AddIntConstant(m, "APP_CODE_DAC", APP_CODE_DAC) != 0)
        return;
    if (PyModule_AddIntConstant(m, "APP_CODE_PWM", APP_CODE_PWM) != 0)
        return;
    if (PyModule_AddIntConstant(m, "APP_CODE_ADC", APP_CODE_ADC) != 0)
        return;
    */

    /* DAQ_DAC_CHANNELS enum */
    /*
    if (PyModule_AddIntConstant(m, "DAC_CHAN_0", DAC_CHAN_0) != 0)
        return;
    if (PyModule_AddIntConstant(m, "DAC_CHAN_1", DAC_CHAN_1) != 0)
        return;
    if (PyModule_AddIntConstant(m, "DAC_CHAN_2", DAC_CHAN_2) != 0)
        return;
    if (PyModule_AddIntConstant(m, "DAC_CHAN_3", DAC_CHAN_3) != 0)
        return;
    if (PyModule_AddIntConstant(m, "DAC_CHAN_4", DAC_CHAN_4) != 0)
        return;
    if (PyModule_AddIntConstant(m, "DAC_CHAN_5", DAC_CHAN_5) != 0)
        return;
    if (PyModule_AddIntConstant(m, "DAC_CHAN_6", DAC_CHAN_6) != 0)
        return;
    if (PyModule_AddIntConstant(m, "DAC_CHAN_7", DAC_CHAN_7) != 0)
        return;
    */
}

static PyObject *py_usbhid_buddy_init(PyObject *self, PyObject *args)
{
    /*
    PyObject *ret;
    uint8_t mode;

    if (!PyArg_ParseTuple(args, "b", &mode)) {
        return NULL;
    }

    device = usbhid_buddy_init(mode);

    if (!device) {
        PyErr_SetString(PyExc_RuntimeError,
                    "Could not initialize usbhid buddy device");
        return NULL;
    }

    ret = Py_BuildValue("i", 0);
    return ret;
    */
}

static PyObject *py_usbhid_write_packet(PyObject *self, PyObject *args)
{
    /*
    PyObject *in_tuple;
    PyObject *ret;
    PyObject *hold_item;
    PyObject *temp;
    uint8_t hold_buffer[MAX_OUT_SIZE];
    uint32_t tuple_size;
    uint32_t i;
    unsigned long long_value;

    if (!PyArg_ParseTuple(args, "O", &in_tuple)) {
        return NULL;
    }

    if (!PyTuple_Check(in_tuple)) {
        PyErr_SetString(PyExc_RuntimeError,
                    "usbhid_write_packet only accepts tuple buffer as argument");
        return NULL;
    }

    tuple_size = PyTuple_Size(in_tuple);
    i = 0;

    while (i < tuple_size) {
        hold_item = PyTuple_GetItem(in_tuple, i);
    
        #if PY_MAJOR_VERSION >= 3
        
        if(!PyLong_Check(hold_item)) {
            PyErr_SetString(PyExc_RuntimeError,
                    "usbhid_write_packet non-integer in tuple argument");
            return NULL;
        }

        #else
        
        if(!PyInt_Check(hold_item)) {
            PyErr_SetString(PyExc_RuntimeError,
                    "usbhid_write_packet non-integer in tuple argument");
            return NULL;
        }
        
        #endif
    
        temp = PyNumber_Long(hold_item);
        long_value = PyLong_AsUnsignedLong(temp);
        hold_buffer[i] = (uint8_t) PyLong_AsUnsignedLong(temp);
        i++;
    }

    if (usbhid_write_packet(device, 
            (uint8_t *) &hold_buffer[0], MAX_OUT_SIZE) != 0) {
        PyErr_SetString(PyExc_RuntimeError,
                    "Could not write usbhid buddy packet");
        return NULL;
    }

    ret = Py_BuildValue("i", 0);
    return ret;
    */
}

static PyObject *py_usbhid_read_packet(PyObject *self, PyObject *args)
{
    /*
    return NULL;
    */
}

static PyObject *py_usbhid_buddy_cleanup(PyObject *self, PyObject *args)
{
    /*
    PyObject *ret;

    if (!device) {
        PyErr_SetString(PyExc_RuntimeError,
                    "usbhid buddy not initialized");
        return NULL;
    }

    usbhid_buddy_cleanup(device);

    ret = Py_BuildValue("i", 0);
    return ret;
    */
}
