#include "CurvePanel.h"

namespace HoshioEngine {
	RbfNetwork rbf_network;
	static int parameter_selected = 0;

	constexpr float Sqr(float x) {
		return x * x;
	}

	void AppendToPythonPath(const char* directory)
	{
#ifdef NDEBUG
		// 导入 sys 模块
		PyObject* sys_module = PyImport_ImportModule("sys");
		if (sys_module == NULL) {
			PyErr_Print();
			return;
		}

		// 获取 sys.path（这是一个列表）
		PyObject* sys_path = PyObject_GetAttrString(sys_module, "path");
		if (sys_path == NULL) {
			PyErr_Print();
			Py_DECREF(sys_module);
			return;
		}

		// 将目录路径转换为 Python 字符串
		PyObject* path_str = PyUnicode_DecodeFSDefault(directory);
		if (path_str == NULL) {
			PyErr_Print();
			Py_DECREF(sys_path);
			Py_DECREF(sys_module);
			return;
		}

		// 将路径添加到 sys.path
		if (PyList_Append(sys_path, path_str) == -1) {
			PyErr_Print();
		}

		// 清理引用
		Py_DECREF(path_str);
		Py_DECREF(sys_path);
		Py_DECREF(sys_module);
#endif // !
	}

	Eigen::MatrixXf LeastSquares(const std::vector<Eigen::Vector2f>& in_pos, int m, float lambda = 0.0f) {
		const int n = in_pos.size();
		Eigen::MatrixXf X(n, m + 1);
		for (size_t i = 0; i < n; i++) {
			float temp = 1.0f;
			for (size_t j = 0; j < m + 1; j++) {
				X(i, j) = temp;
				temp *= in_pos[i].x();
			}
		}

		Eigen::MatrixXf Y(n, 1);
		for (size_t i = 0; i < n; i++) {
			Y(i) = in_pos[i].y();
		}

		Eigen::MatrixXf I = Eigen::MatrixXf::Identity(m + 1, m + 1);

		Eigen::VectorXf w = (X.transpose() * X + lambda * I).inverse() * X.transpose() * Y;

		return w;
	}

	Eigen::MatrixXf GradientDescent(const std::vector<Eigen::Vector2f>& in_pos, std::vector<float>& xs, float mean, float stddev,
		int m, int iter_times = 1000, float lr = 0.1f) {
		const int n = static_cast<int>(in_pos.size());
		Eigen::MatrixXf w = Eigen::MatrixXf::Zero(m + 1, 1);

		// 构造 Y
		Eigen::MatrixXf Y(n, 1);
		for (int i = 0; i < n; ++i) Y(i, 0) = in_pos[i].y();

		// 构造 X（多项式特征，基于标准化后的 x）
		Eigen::MatrixXf X(n, m + 1);
		for (int i = 0; i < n; ++i) {
			float x = (xs[i] - mean) / stddev;
			float temp = 1.0f;
			for (int j = 0; j <= m; ++j) {
				X(i, j) = temp;
				temp *= x;
			}
		}

		const float lambda = 0.0f; // 需要正则时设 > 0，并对偏置例外处理
		for (int it = 0; it < iter_times; ++it) {
			Eigen::MatrixXf r = X * w - Y;                     // 残差
			Eigen::MatrixXf grad = (X.transpose() * r) / n;    // 平均梯度
			grad += lambda * w;                                 // L2（可选）
			Eigen::MatrixXf w_new = w - lr * grad;

			// 收敛判据（相对变化）
			if ((w_new - w).norm() <= 1e-6f * std::max(1.0f, w.norm())) {
				w = w_new;
				break;
			}
			w = w_new;
		}
		return w;
	}

	std::vector<Eigen::Vector2f>
		TrainRbfNetwork(const std::vector<Eigen::Vector2f>& in_pos, int num_middle, int epochs, float lb, float rb,
			float step, bool& training) {
		auto res = rbf_network.Train(in_pos, num_middle, epochs, lb, rb, step);
		std::cout << "Train Finished!" << std::endl;
		training = false;
		return res;
	}

	void GenerateParamMapping(const std::vector<Eigen::Vector2f>& in_pos, const std::vector<float>& t, std::vector<Eigen::Vector2f>& t_x, std::vector<Eigen::Vector2f>& t_y) {
		const int n = in_pos.size();
		for (int i = 0; i < n; i++) {
			t_x[i] = Eigen::Vector2f(t[i], in_pos[i].x());
			t_y[i] = Eigen::Vector2f(t[i], in_pos[i].y());
		}
	}

	std::vector<Eigen::Vector2f> CombineXY(std::vector<Eigen::Vector2f>& pos_x, std::vector<Eigen::Vector2f>& pos_y) {
		const int n = pos_x.size();
		std::vector<Eigen::Vector2f> result(n);
		for (int i = 0; i < n; i++) {
			result[i] = Eigen::Vector2f(pos_x[i].y(), pos_y[i].y());
		}
		return result;
	}


	ImVec2 g_canvas_pos_lu = { 0.0f, 0.0f };
	ImVec2 g_canvas_pos_rb = { 0.0f, 0.0f };

	void PlotLineSegments(const std::vector<Eigen::Vector2f>& poss, ImDrawList* draw_list, ImU32 line_col, ImU32 point_col, bool plot_dot = true) {
		//draw line segments
		for (size_t i = 1; i < poss.size(); i++) {
			draw_list->AddLine({ poss[i - 1].x() + g_canvas_pos_lu.x, g_canvas_pos_rb.y - poss[i - 1].y() },
				{ poss[i].x() + g_canvas_pos_lu.x, g_canvas_pos_rb.y - poss[i].y() },line_col);
		}
		//draw point
		if (plot_dot) {
			for (auto& pos : poss) {
				draw_list->AddCircleFilled({ pos.x() + g_canvas_pos_lu.x, g_canvas_pos_rb.y - pos.y() }, 5.0f, point_col);
				draw_list->AddCircle({ pos.x() + g_canvas_pos_lu.x, g_canvas_pos_rb.y - pos.y() }, 5.0f, point_col);
			}
		}
	}

	void PlotCanvasGrid(float spacing, ImDrawList* draw_list, ImU32 line_col) {
		for (float temp = spacing + g_canvas_pos_lu.x; temp < g_canvas_pos_rb.x; temp += spacing) {
			draw_list->AddLine({ temp, g_canvas_pos_lu.y }, { temp, g_canvas_pos_rb.y }, line_col);
		}

		for (float temp = g_canvas_pos_rb.y - spacing; temp > g_canvas_pos_lu.y; temp -= spacing) {
			draw_list->AddLine({ g_canvas_pos_lu.x, temp }, { g_canvas_pos_rb.x, temp }, line_col);
		}
	}

	CurvePanel::CurvePanel()
	{
#ifdef NDEBUG
		Py_Initialize();
		AppendToPythonPath("test/TestCurve");
		if (!rbf_network.Initialize()) {
			std::cout << "Failed to initialize RBF Network" << std::endl;
			throw std::runtime_error("Failed to initialize RBF Network");
		}
#endif // NDEBUG
	}

	CurvePanel::~CurvePanel()
	{
#ifdef NDEBUG
		rbf_network.Finalize();
		std::cout << "Final" << std::endl;
		if (Py_FinalizeEx() < 0) {
			std::cout << "Failed when Py_FinalizeEx()" << std::endl;
			throw std::runtime_error("Failed when Py_FinalizeEx()");
		}
#endif // NDEBUG
	}

	std::vector<Eigen::Vector2f> CurvePanel::InterpolationPolygon(const std::vector<Eigen::Vector2f>& in_pos, float lb, float rb, float step)
	{
		std::vector<Eigen::Vector2f> result;

		if (inter_poly.is_least_square) {
			const size_t n = in_pos.size();
			Eigen::MatrixXf X(n, n);
			for (size_t i = 0; i < n; i++) {
				float temp = 1;
				for (size_t j = 0; j < n; j++) {
					X(i, j) = temp;
					temp *= in_pos[i].x();
				}
			}
			Eigen::MatrixXf Y(n, 1);
			for (size_t i = 0; i < n; i++)
				Y(i, 0) = in_pos[i].y();
			Eigen::VectorXf A = X.inverse() * Y;

			for (float x = lb; x <= rb; x += step) {
				float y = 0, temp = 1.0f;
				for (size_t i = 0; i < n; i++) {
					y += A(i) * temp;
					temp *= x;
				}
				result.emplace_back(x, y);
			}

		}
		else {
			for (float x = lb; x <= rb; x += step) {
				float y = 0;
				for (int i = 0; i < in_pos.size(); i++) {
					float temp = in_pos[i].y();
					for (int j = 0; j < in_pos.size(); j++) {
						if (i != j) {
							temp = temp * (x - in_pos[j].x()) / (in_pos[i].x() - in_pos[j].x());
						}
					}
					y += temp;
				}
				result.emplace_back(x, y);
			}
		}
		return result;
	}
	std::vector<Eigen::Vector2f> CurvePanel::InterpolationGauss(const std::vector<Eigen::Vector2f>& in_pos, float sigma2, int m, float lb, float rb, float step)
	{
		return std::vector<Eigen::Vector2f>();
	}
	std::vector<Eigen::Vector2f> CurvePanel::ApproximationPolygon(const std::vector<Eigen::Vector2f>& in_pos, int m, float lb, float rb, float step)
	{
		const int n = in_pos.size();
		std::vector<Eigen::Vector2f> result;
		m = std::min(m, std::max(n - 1, 0));
		Eigen::VectorXf w;

		std::vector<float> xs(n);
		for (int i = 0; i < n; ++i) xs[i] = in_pos[i].x();
		float mean = 0.f, var = 0.f;
		for (float v : xs) mean += v;
		mean /= n;
		for (float v : xs) { float d = v - mean; var += d * d; }
		float stddev = std::sqrt(var / std::max(1, n - 1));
		if (stddev == 0.f) stddev = 1.f;

		if (approx_poly.is_grad)
			w = GradientDescent(in_pos, xs, mean, stddev, m, 1000, approx_poly.lr);
		else
			w = LeastSquares(in_pos, m);
		for (float x = lb; x <= rb; x += step) {
			float y = 0, temp = 1;
			float _x = approx_poly.is_grad ? (x - mean) / stddev : x;
			for (size_t i = 0; i < m + 1; i++) {
				y += temp * w(i);
				temp *= _x;
			}
			result.emplace_back(x, y);
		}
		return result;
	}
	std::vector<Eigen::Vector2f> CurvePanel::ApproximationNormalized(const std::vector<Eigen::Vector2f>& in_pos, int m, float lambda, float lb, float rb, float step)
	{
		return std::vector<Eigen::Vector2f>();
	}

	void CurvePanel::UniformParameterization(const std::vector<Eigen::Vector2f>& in_pos)
	{
		const int n = in_pos.size();
		std::vector<Eigen::Vector2f> t_x(n);
		std::vector<Eigen::Vector2f> t_y(n);

		std::vector<float> t(n);
		for (int i = 1; i < n; i++) {
			t[i] = t[i - 1] + curve_param.t_const;
		}

		GenerateParamMapping(in_pos, t, t_x, t_y);

		std::vector<Eigen::Vector2f> pos_x = InterpolationPolygon(t_x, t[0], t[n - 1] + curve_param.step, curve_param.step);
		std::vector<Eigen::Vector2f> pos_y = InterpolationPolygon(t_y, t[0], t[n - 1] + curve_param.step, curve_param.step);
			
		curve_param.pos = CombineXY(pos_x, pos_y);

		return;
	}

	void CurvePanel::ArcLengthParameterization(const std::vector<Eigen::Vector2f>& in_pos)
	{
		const int n = in_pos.size();
		std::vector<Eigen::Vector2f> t_x(n);
		std::vector<Eigen::Vector2f> t_y(n);

		std::vector<float> t(n);
		for (int i = 1; i < n; i++) {
			t[i] = t[i - 1] + ((in_pos[i] - in_pos[i-1]).norm());
		}

		for (int i = 1; i < n; i++) {
			t[i] =  t[i] / t[n - 1] * (curve_param.t_const * n);
		}

		GenerateParamMapping(in_pos, t, t_x, t_y);

		std::vector<Eigen::Vector2f> pos_x = InterpolationPolygon(t_x, t[0], t[n - 1] + curve_param.step, curve_param.step);
		std::vector<Eigen::Vector2f> pos_y = InterpolationPolygon(t_y, t[0], t[n - 1] + curve_param.step, curve_param.step);

		curve_param.pos = CombineXY(pos_x, pos_y);

		return;
	}

	void CurvePanel::CenterParameterization(const std::vector<Eigen::Vector2f>& in_pos)
	{
		const int n = in_pos.size();
		std::vector<Eigen::Vector2f> t_x(n);
		std::vector<Eigen::Vector2f> t_y(n);

		std::vector<float> t(n);
		for (int i = 1; i < n; i++) {
			t[i] = t[i - 1] + sqrtf(((in_pos[i] - in_pos[i - 1]).norm()));
		}

		for (int i = 1; i < n; i++) {
			t[i] = t[i] / t[n - 1] * (curve_param.t_const * n);
		}

		GenerateParamMapping(in_pos, t, t_x, t_y);

		std::vector<Eigen::Vector2f> pos_x = InterpolationPolygon(t_x, t[0], t[n - 1] + curve_param.step, curve_param.step);
		std::vector<Eigen::Vector2f> pos_y = InterpolationPolygon(t_y, t[0], t[n - 1] + curve_param.step, curve_param.step);

		curve_param.pos = CombineXY(pos_x, pos_y);

		return;
	}

	void CurvePanel::CurveParameterization(const std::vector<Eigen::Vector2f>& in_pos, ParameterizationOption option)
	{
		switch (option)
		{
		case HoshioEngine::ParameterizationOption::uniform:
			UniformParameterization(in_pos);
			break;
		case HoshioEngine::ParameterizationOption::arc_length:
			ArcLengthParameterization(in_pos);
			break;
		case HoshioEngine::ParameterizationOption::center:
			CenterParameterization(in_pos);
			break;
		default:
			break;
		}
		return;
	}

	void CurvePanel::Render()
	{
		//define the step and lb, rb
		float step = 20.0f;
		float lb = step;
		float rb = g_canvas_pos_rb.x - step - g_canvas_pos_lu.x;

		if (ImGui::Begin("Config")) {
			if (ImGui::TreeNode("Interpolation and fitting")) {
				ImGui::Checkbox("Interpolation - Polygon (Red)", &inter_poly.visible);
				ImGui::SameLine();
				if (ImGui::Checkbox("Use least square", &inter_poly.is_least_square)) {
					inter_poly.update = true;
				}

				ImGui::Checkbox("Fitting - Polygon (Blue)", &approx_poly.visible);
				ImGui::SameLine();
				if (ImGui::Checkbox("Use Gradient Desent", &approx_poly.is_grad)) {
					approx_poly.update = true;
				}
				ImGui::SameLine();
				ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.2f);
				ImGui::InputInt("m##2", &approx_poly.m_temp);
				approx_poly.m_temp = std::clamp(approx_poly.m_temp, 0, std::max<int>(0, approx_poly.pos.size() - 1));
				if (approx_poly.m_temp != approx_poly.m) {
					approx_poly.m = approx_poly.m_temp;
					approx_poly.update = true;
				}
				ImGui::SameLine();
				ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.2f);
				ImGui::SliderFloat("lr##2", &approx_poly.lr, 0.001, 0.5);

#ifdef NDEBUG
				ImGui::Checkbox("RBF Network - Gauss (Pink)", &rbf_nn.visible);
				ImGui::SameLine();
				rbf_nn.update = ImGui::Button("train");
				if (rbf_nn.training) {
					rbf_nn.update = false;
				}

				ImGui::SameLine();
				ImGui::LabelText("##1", "%s", rbf_nn.training ? "(training)" : "");
				ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.2f);
				ImGui::LabelText("##2", "# middle: %d", rbf_nn.num_middle);
				ImGui::SameLine();
				ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.2f);
				ImGui::InputInt("##3", &rbf_nn.num_middle_temp);
				rbf_nn.num_middle_temp = std::max(1, rbf_nn.num_middle_temp);
				ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.2f);
				ImGui::LabelText("##4", "# epochs: %d", rbf_nn.epochs);
				ImGui::SameLine();
				ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.2f);
				ImGui::InputInt("##5", &rbf_nn.epochs_temp);
				rbf_nn.epochs_temp = std::max(1, rbf_nn.epochs_temp);
#endif // NDEBUG
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Curve Parameterization")) {
				ImGui::Checkbox("Parameterization - Polygon (Orange)", &curve_param.visible);
				ImGui::BeginGroup();
				if (ImGui::RadioButton("uniform", &parameter_selected, 0)) {
					curve_param.option = ParameterizationOption::uniform;
					curve_param.update = true;
				}
				ImGui::SameLine();
				if (ImGui::RadioButton("arc_length", &parameter_selected, 1)) {
					curve_param.option = ParameterizationOption::arc_length;
					curve_param.update = true;
				}
				ImGui::SameLine();
				if (ImGui::RadioButton("center", &parameter_selected, 2)) {
					curve_param.option = ParameterizationOption::center;
					curve_param.update = true;
				}
				ImGui::EndGroup();
				if (ImGui::SliderFloat("step", &curve_param.step_temp, 5, 50)) {
					curve_param.update = true;
				}
				if (curve_param.option == ParameterizationOption::uniform) {
					if (ImGui::SliderFloat("t_const", &curve_param.t_cosnt_temp, 0.1, 10)) {
						curve_param.update = true;
					}
				}
				ImGui::TreePop();
			}
			
			ImGui::End();
		}

		if (ImGui::Begin("Canvas")) {
			//calculate the left, upper, right, bottom position of the canvas
			g_canvas_pos_lu = ImGui::GetCursorScreenPos();
			ImVec2 canvas_size = ImGui::GetContentRegionAvail();
			if (canvas_size.x < 50.f)
				canvas_size.x = 50.0f;
			if (canvas_size.y < 50.0f)
				canvas_size.x = 50.0f;
			g_canvas_pos_rb = ImVec2(g_canvas_pos_lu.x + canvas_size.x, g_canvas_pos_lu.y + canvas_size.y);

			//get the imgui io to fetch the input 
			ImGuiIO& io = ImGui::GetIO();
			//get the imgui window drawlist to draw item in the canvas
			ImDrawList* drawlist = ImGui::GetWindowDrawList();
			//paint the canvas to make it visible
			drawlist->AddRectFilled(g_canvas_pos_lu, g_canvas_pos_rb, IM_COL32(50, 50, 50, 255));
			PlotCanvasGrid(80.0f, drawlist, IM_COL32(120, 120, 120, 255));
			drawlist->AddRect(g_canvas_pos_lu, g_canvas_pos_rb, IM_COL32(80, 80, 80, 255), 0.0f, 0, 10.0f);


			//create a invisible button and add hover test for it
			ImGui::InvisibleButton("canvas", canvas_size);
			const bool is_hovered = ImGui::IsItemHovered();
			//if mouse hover on the canvas and left mouse button Clicked
			if (is_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
				//get the mouse click pos related to the left and bottom position of canvas
				in_pos.emplace_back(io.MousePos.x - g_canvas_pos_lu.x, g_canvas_pos_rb.y - io.MousePos.y);
				in_pos_v2.emplace_back(io.MousePos.x - g_canvas_pos_lu.x, g_canvas_pos_rb.y - io.MousePos.y);
				//sort the position
				std::sort(in_pos.begin(), in_pos.begin() + in_pos.size(), [](const Eigen::Vector2f& a, const Eigen::Vector2f& b) {
					return a.x() < b.x();
					});
				//interpolate base on the in_pos
				inter_poly.update = true;
				approx_poly.update = true;
				curve_param.update = true;
			}
			else if (is_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
				Eigen::Vector2f mouse_pos = {io.MousePos.x - g_canvas_pos_lu.x, g_canvas_pos_rb.y - io.MousePos.y};
				size_t index = 0;
				float min_dist = std::numeric_limits<float>::max();
				//find the closest point
				for (int i = 0; i < in_pos.size(); i++) {
					float dist = (mouse_pos - in_pos[i]).squaredNorm();
					if (dist < min_dist) {
						min_dist = dist;
						index = i;
					}
				}

				//determine wheter the mouse clicked position is on the point exsiting
				if (min_dist <= 100.0f) {
					in_pos.erase(in_pos.begin() + index, in_pos.begin() + index + 1);
					//reinterpolate base on the updated in_pos	
					inter_poly.update = true;
					approx_poly.update = true;
				}

				min_dist = std::numeric_limits<float>::max();
				//find the closest point
				for (int i = 0; i < in_pos_v2.size(); i++) {
					float dist = (mouse_pos - in_pos_v2[i]).squaredNorm();
					if (dist < min_dist) {
						min_dist = dist;
						index = i;
					}
				}

				//determine wheter the mouse clicked position is on the point exsiting
				if (min_dist <= 100.0f) {
					in_pos_v2.erase(in_pos_v2.begin() + index, in_pos_v2.begin() + index + 1);
					//reinterpolate base on the updated in_pos	
					curve_param.update = true;
				}
			}

			if (inter_poly.update) {
				inter_poly.pos = InterpolationPolygon(in_pos, lb, rb, step);
				inter_poly.update = false;
			}

			if (approx_poly.update) {
				approx_poly.pos = ApproximationPolygon(in_pos, approx_poly.m, lb, rb, step);
				approx_poly.update = false;
			}

			if(curve_param.update)

#ifdef NDEBUG
			if (rbf_nn.update) {
				rbf_nn.num_middle = rbf_nn.num_middle_temp;
				rbf_nn.epochs = rbf_nn.epochs_temp;
				rbf_nn.update = false;
				rbf_nn.training = true;
				rbf_nn.pos = TrainRbfNetwork(in_pos, rbf_nn.num_middle, rbf_nn.epochs, lb, rb, step, rbf_nn.training);
			}

			if (!rbf_nn.training && !rbf_nn.finished) {
				rbf_nn.finished = true;
			}
#endif
			if (curve_param.update) {
				curve_param.t_const = curve_param.t_cosnt_temp;
				curve_param.step = curve_param.step_temp;
				CurveParameterization(in_pos_v2, curve_param.option);
				curve_param.update = false;
			}

			//draw curve
			if (inter_poly.visible) {
				PlotLineSegments(inter_poly.pos, drawlist, IM_COL32(255, 50, 50, 255), IM_COL32(255, 80, 80, 255));
			}
			if (approx_poly.visible) {
				PlotLineSegments(approx_poly.pos, drawlist, IM_COL32(50, 50, 255, 255), IM_COL32(80, 80, 255, 255));
			}
#ifdef NDEBUG
			if (rbf_nn.visible) {
				PlotLineSegments(rbf_nn.pos, drawlist, IM_COL32(178, 102, 255, 255), IM_COL32(178, 102, 255, 255));
			}
#endif
			if (curve_param.visible) {
				PlotLineSegments(curve_param.pos, drawlist, IM_COL32(255, 127, 80, 255), IM_COL32(255, 127, 80, 255), false);
			}
			PlotLineSegments(in_pos, drawlist, IM_COL32(255, 255, 255, 0), IM_COL32(255, 255, 255, 255));
			ImGui::End();

		}
	}



	}