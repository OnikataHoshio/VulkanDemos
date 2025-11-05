#ifndef _RBF_NETWORK_H_
#define _RBF_NETWORK_H_

#include "VulkanCommon.h"

namespace HoshioEngine {
	class RbfNetwork {
	public:
		bool Initialize();
		bool Finalize();

		std::vector<Eigen::Vector2f> Train(const std::vector<Eigen::Vector2f>& in_pos, int num_middle, int epochs, float lb, float rb, float step);

	private:
#ifdef NDEBUG
		PyObject* py_module = nullptr;
		PyObject* py_train_function = nullptr;
#endif // !
	};
}

#endif // !_RBF_NETWORK_H_
