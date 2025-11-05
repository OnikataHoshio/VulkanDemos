#include "RbfNetwork.h"

namespace HoshioEngine {
	bool RbfNetwork::Initialize()
	{
#ifdef NDEBUG
		PyObject* module_name = PyUnicode_DecodeFSDefault("rbf_nn");
		py_module = PyImport_Import(module_name);
		Py_DECREF(module_name);
		if (!py_module) {
			std::cerr << "Failed to load python module rbf_nn" << std::endl;
			PyErr_Print();
			return false;
		}
		py_train_function = PyObject_GetAttrString(py_module, "train");
		if (!py_train_function || !PyCallable_Check(py_train_function)) {
			std::cerr << "Failed to load python function 'train' " << std::endl;
			PyErr_Print();
			return false;
		}
#endif // NDEBUG
		return true;
	}
	bool RbfNetwork::Finalize()
	{
#ifdef NDEBUG
		Py_DECREF(py_module);
		Py_DECREF(py_train_function);
#endif
		return true;
	}
	std::vector<Eigen::Vector2f> RbfNetwork::Train(const std::vector<Eigen::Vector2f>& in_pos, int num_middle, int epochs, float lb, float rb, float step)
	{
#ifdef NDEBUG
		const int n_pos_size = in_pos.size();

		PyObject* func_args = PyTuple_New(5);

		PyObject* x_list = PyList_New(n_pos_size);
		PyObject* y_list = PyList_New(n_pos_size);
		for (size_t i = 0; i < n_pos_size; i++) {
			PyList_SetItem(x_list, i, PyFloat_FromDouble(in_pos[i].x()));
			PyList_SetItem(y_list, i, PyFloat_FromDouble(in_pos[i].y()));
		}

		Py_ssize_t size = PyList_Size(x_list);
		std::cout << "PyList size: " << size << std::endl;
		for (Py_ssize_t i = 0; i < size; ++i) {
			PyObject* item = PyList_GetItem(x_list, i);
			double value = PyFloat_AsDouble(item);
			std::cout << "Item[" << i << "] = " << value << std::endl;
		}


		std::vector<float> x_pred;
		for (float x = lb; x < rb; x += step) {
			x_pred.push_back(x);
		}

		PyObject* xp_list = PyList_New(x_pred.size());

		for (size_t i = 0; i < x_pred.size(); i++) {
			PyList_SetItem(xp_list, i, PyFloat_FromDouble(x_pred[i]));
		}

		PyTuple_SetItem(func_args, 0, x_list);
		PyTuple_SetItem(func_args, 1, y_list);
		PyTuple_SetItem(func_args, 2, PyLong_FromLong(num_middle));
		PyTuple_SetItem(func_args, 3, PyLong_FromLong(epochs));
		PyTuple_SetItem(func_args, 4, xp_list);

		PyObject* fn_ret = PyObject_CallObject(py_train_function, func_args);

		if (!fn_ret) {
			std::cout << "Failed to call Python function 'train'" << std::endl;
			PyErr_Print();
			Py_DECREF(func_args);
			Py_DECREF(x_list);
			Py_DECREF(y_list);
			Py_DECREF(xp_list);
			return {};
		}

		std::vector<Eigen::Vector2f> result;
		for (size_t i = 0; i < x_pred.size(); i++) {
			result.emplace_back(x_pred[i], PyFloat_AsDouble(PyList_GetItem(fn_ret, i)));
		}

		Py_DECREF(func_args);
		Py_DECREF(x_list);
		Py_DECREF(y_list);
		Py_DECREF(xp_list);
		Py_DECREF(fn_ret);
		return result;
#endif
		return {};
	}
}

