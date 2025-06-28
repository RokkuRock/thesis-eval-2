TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node) {
  TF_LITE_ENSURE_EQ(context, NumInputs(node), 2);
  TF_LITE_ENSURE_EQ(context, NumOutputs(node), 1);
  OpContext op_context(context, node);
  TF_LITE_ENSURE_OK(context, InitializeTemporaries(context, node, &op_context));
  OpData* op_data = reinterpret_cast<OpData*>(node->user_data);
  bool adj_x = op_context.params->adj_x;
  bool adj_y = op_context.params->adj_y;
  const TfLiteTensor* lhs_data = GetInput(context, node, kInputLHSTensor);
  const TfLiteTensor* rhs_data = GetInput(context, node, kInputRHSTensor);
  TfLiteTensor* output = GetOutput(context, node, kOutputTensor);
  if (lhs_data->type == kTfLiteInt8) {
    double real_multiplier = 0.0;
    TF_LITE_ENSURE_STATUS(GetQuantizedConvolutionMultipler(
        context, lhs_data, rhs_data, output, &real_multiplier));
    int exponent;
    QuantizeMultiplier(real_multiplier, &op_data->output_multiplier, &exponent);
    op_data->output_shift = exponent;
    op_data->output_activation_min = std::numeric_limits<int8_t>::min();
    op_data->output_activation_max = std::numeric_limits<int8_t>::max();
  }
  TF_LITE_ENSURE(context, lhs_data->type == kTfLiteFloat32 ||
                              lhs_data->type == kTfLiteInt8);
  TF_LITE_ENSURE(context, rhs_data->type == kTfLiteFloat32 ||
                              rhs_data->type == kTfLiteInt8);
  TF_LITE_ENSURE(context, NumDimensions(lhs_data) >= 2);
  TF_LITE_ENSURE(context, NumDimensions(lhs_data) <= 4);
  TF_LITE_ENSURE(context, NumDimensions(rhs_data) >= 2);
  TF_LITE_ENSURE(context, NumDimensions(rhs_data) <= 4);
  const int lhs_rank = NumDimensions(lhs_data);
  const int rhs_rank = NumDimensions(rhs_data);
  const int output_rank = std::max(lhs_rank, rhs_rank);
  const RuntimeShape extended_lhs_shape =
      RuntimeShape::ExtendedShape(output_rank, GetTensorShape(lhs_data));
  const RuntimeShape extended_rhs_shape =
      RuntimeShape::ExtendedShape(output_rank, GetTensorShape(rhs_data));
  for (int i = 0; i < output_rank - 2; ++i) {
    const int lhs_dim = extended_lhs_shape.Dims(i);
    const int rhs_dim = extended_rhs_shape.Dims(i);
    if (lhs_dim != rhs_dim) {
      if (lhs_dim != 1) {
        TF_LITE_ENSURE_EQ(context, rhs_dim, 1);
      }
    }
  }
  int accum_dim_lhs = adj_x ? extended_lhs_shape.Dims(output_rank - 2)
                            : extended_lhs_shape.Dims(output_rank - 1);
  int accum_dim_rhs = adj_y ? extended_rhs_shape.Dims(output_rank - 1)
                            : extended_rhs_shape.Dims(output_rank - 2);
  TF_LITE_ENSURE_EQ(context, accum_dim_lhs, accum_dim_rhs);
  TfLiteStatus status =
      ResizeOutputTensor(context, extended_lhs_shape, extended_rhs_shape, adj_x,
                         adj_y, output_rank, output);
  return status;
}