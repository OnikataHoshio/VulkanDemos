#ifndef _CURVE_PANEL_H_
#define _CURVE_PANEL_H_

#include "Engine/Panel/Editor/EditorPanel.h"

#include "RbfNetwork.h"

namespace HoshioEngine {
	enum class ParameterizationOption {
		uniform,
		arc_length,
		center
	};

	class CurvePanel : public EditorPanel{
	public:
		CurvePanel();
		~CurvePanel();

		std::vector<Eigen::Vector2f>
			InterpolationPolygon(const std::vector<Eigen::Vector2f>& in_pos, float lb, float rb, float step);

		std::vector<Eigen::Vector2f>
			InterpolationGauss(const std::vector<Eigen::Vector2f>& in_pos, float sigma2, int m,
				float lb, float rb, float step);

		std::vector<Eigen::Vector2f>
			ApproximationPolygon(const std::vector<Eigen::Vector2f>& in_pos, int m, float lb, float rb, float step);

		std::vector<Eigen::Vector2f>
			ApproximationNormalized(const std::vector<Eigen::Vector2f>& in_pos, int m, float lambda,
				float lb, float rb, float step);

		void UniformParameterization(const std::vector<Eigen::Vector2f>& in_pos);
		void ArcLengthParameterization(const std::vector<Eigen::Vector2f>& in_pos);
		void CenterParameterization(const std::vector<Eigen::Vector2f>& in_pos);
		void CurveParameterization(const std::vector<Eigen::Vector2f>& in_pos, ParameterizationOption option);


		std::vector<Eigen::Vector2f> in_pos;
		std::vector<Eigen::Vector2f> in_pos_v2;
		struct {
			std::vector<Eigen::Vector2f> pos;
			bool visible = false;
			bool update = false;
			bool is_least_square = false;
		}inter_poly;

		struct {
			std::vector<Eigen::Vector2f> pos;
			int m = 0;
			int m_temp = 0;
			float lr = 0.1;
			bool visible = false;
			bool update = false;
			bool is_grad = false;
		}approx_poly;

		struct {
			std::vector<Eigen::Vector2f> pos;
			int num_middle = 0;
			int num_middle_temp = 0;
			int epochs = 2000;
			int epochs_temp = 2000;
			bool visible = false;
			bool update = false;
			bool training = false;
			bool finished = true;
			std::future<std::vector<Eigen::Vector2f>> future;
		} rbf_nn;

		struct {
			std::vector<Eigen::Vector2f> pos;
			float step = 0.1;
			float step_temp = 0.1;
			bool visible = false;
			bool update = false;
			ParameterizationOption option = ParameterizationOption::uniform;
			float t_const = 1.0f;
			float t_cosnt_temp = 1.0f;
		} curve_param;

		// Í¨¹ý EditorPanel ¼Ì³Ð
		void Render() override;

	};
}

#endif // !_CURVE_PANEL_H_
