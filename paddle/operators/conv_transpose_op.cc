/* Copyright (c) 2016 PaddlePaddle Authors. All Rights Reserve.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. */

#include "paddle/operators/conv_transpose_op.h"

namespace paddle {
namespace operators {

void ConvTransposeOp::InferShape(framework::InferShapeContext* ctx) const {
  PADDLE_ENFORCE(ctx->HasInput("Input"),
                 "Input(Input) of ConvTransposeOp should not be null.");
  PADDLE_ENFORCE(ctx->HasInput("Filter"),
                 "Input(Filter) of ConvTransposeOp should not be null.");
  PADDLE_ENFORCE(ctx->HasOutput("Output"),
                 "Output(Output) of ConvTransposeOp should not be null.");

  auto in_dims = ctx->GetInputDim("Input");
  auto filter_dims = ctx->GetInputDim("Filter");
  std::vector<int> strides = ctx->Attrs().Get<std::vector<int>>("strides");
  std::vector<int> paddings = ctx->Attrs().Get<std::vector<int>>("paddings");

  PADDLE_ENFORCE(in_dims.size() == 4 || in_dims.size() == 5,
                 "ConvTransposeOp intput should be 4-D or 5-D tensor.");
  PADDLE_ENFORCE_EQ(in_dims.size(), filter_dims.size(),
                    "ConvTransposeOp input dimension and filter dimension "
                    "should be the same.");
  PADDLE_ENFORCE(in_dims.size() - strides.size() == 2U,
                 "ConvTransposeOp input dimension and strides dimension should "
                 "be consistent.");
  PADDLE_ENFORCE_EQ(paddings.size(), strides.size(),
                    "ConvTransposeOp paddings dimension and strides "
                    "dimension should be the same.");
  PADDLE_ENFORCE_EQ(in_dims[1], filter_dims[0],
                    "In ConvTransposeOp, The input channel should be the same "
                    "as the number of filters.");

  std::vector<int64_t> output_shape({in_dims[0], filter_dims[1]});
  for (size_t i = 0; i < strides.size(); ++i) {
    output_shape.push_back((in_dims[i + 2] - 1) * strides[i] - 2 * paddings[i] +
                           filter_dims[i + 2]);
  }
  ctx->SetOutputDim("Output", framework::make_ddim(output_shape));
}

Conv2DTransposeOpMaker::Conv2DTransposeOpMaker(
    framework::OpProto* proto, framework::OpAttrChecker* op_checker)
    : OpProtoAndCheckerMaker(proto, op_checker) {
  AddInput(
      "Input",
      "(Tensor) The input tensor of convolution transpose operator. "
      "The format of input tensor is NCHW. Where N is batch size, C is the "
      "number of input channels, H is the height of the feature, and "
      "W is the width of the feature.");
  AddInput(
      "Filter",
      "(Tensor) The filter tensor of convolution transpose operator. "
      "The format of the filter tensor is MCHW, where M is the number of "
      "input feature channels, C is the number of "
      "output feature channels,"
      "H is the height of the filter, and W is the width of the filter. "
      "We enforce groups number == 1 in the convolution transpose scenario.");
  AddOutput("Output",
            "(Tensor) The output tensor of convolution transpose operator. "
            "The format of output tensor is also NCHW.");
  AddAttr<std::vector<int>>(
      "strides",
      "(vector<int> default:{1, 1}), the strides(h_stride, w_stride) of "
      "convolution transpose operator.")
      .SetDefault({1, 1});
  AddAttr<std::vector<int>>(
      "paddings",
      "(vector<int> default:{0, 0}), the paddings(h_pad, w_pad) of convolution "
      "transpose operator.")
      .SetDefault({0, 0});
  AddComment(R"DOC(
Convolution2D Transpose Operator.

The convolution transpose operation calculates the output based on the input, filter
and strides, paddings, groups parameters. The size of each dimension of the
parameters is checked in the infer-shape.
Input(Input) and output(Output) are in NCHW format. Where N is batchsize, C is the
number of channels, H is the height of the feature, and W is the width of the feature.
Filter(Input) is in MCHW format. Where M is the number of input feature channels,
C is the number of output feature channels, H is the height of the filter,
and W is the width of the filter.
Parameters(strides, paddings) are two elements. These two elements represent height
and width, respectively.
The input(X) size and output(Out) size may be different.

Example:
  Input:
       Input shape: $(N, C_{in}, H_{in}, W_{in})$
       Filter shape: $(C_{in}, C_{out}, H_f, W_f)$
  Output:
       Output shape: $(N, C_{out}, H_{out}, W_{out})$
  Where
  $$
       H_{out} = (H_{in} - 1) * strides[0] - 2 * paddings[0] + H_f \\
       W_{out} = (W_{in} - 1) * strides[1] - 2 * paddings[1] + W_f
  $$
)DOC");
}

Conv3DTransposeOpMaker::Conv3DTransposeOpMaker(
    framework::OpProto* proto, framework::OpAttrChecker* op_checker)
    : OpProtoAndCheckerMaker(proto, op_checker) {
  AddInput("Input",
           "(Tensor) The input tensor of convolution transpose operator."
           "The format of input tensor is NCDHW. Where N is batch size, C is "
           "the number of channels, D is the depth of the feature, H is the "
           "height of the feature, and "
           "W is the width of the feature.");
  AddInput("Filter",
           "(Tensor) The filter tensor of convolution transpose operator."
           "The format of the filter tensor is MCDHW, where M is the number of "
           "input feature channels, C is the number of "
           "output feature channels, D "
           "is the depth of the filter, H is the height of the filter, and "
           "W is the width of the filter."
           "We enforce groups number == 1 and padding == 0 in "
           "the convolution3d transpose scenario.");
  AddOutput("Output",
            "(Tensor) The output tensor of convolution transpose operator."
            "The format of output tensor is also NCDHW."
            "Where N is batch size, C is "
            "the number of channels, D is the depth of the feature, H is the "
            "height of the feature, and W is the width of the feature.");
  AddAttr<std::vector<int>>("strides",
                            "(vector<int> default:{1, 1, 1}), the "
                            "strides{d_stride, h_stride, w_stride} of "
                            "convolution transpose operator.")
      .SetDefault({1, 1, 1});
  AddAttr<std::vector<int>>("paddings",
                            "(vector<int> default:{0, 0, 0}), paddings(d_pad, "
                            "h_pad, w_pad) of convolution transpose operator.")
      .SetDefault({0, 0, 0});
  AddComment(R"DOC(
Convolution3D Transpose Operator.

The convolution transpose operation calculates the output based on the input, filter
and strides, paddings, groups parameters. The size of each dimension of the
parameters is checked in the infer-shape.
Input(Input) and output(Output) are in NCDHW format. Where N is batch size, C is the
number of channels, D is the depth of the feature, H is the height of the feature,
and W is the width of the feature.
Filter(Input) is in MCDHW format. Where M is the number of input feature channels,
C is the number of output feature channels, D is the depth of the filter,H is the
height of the filter, and W is the width of the filter.
Parameters(strides, paddings) are three elements. These three elements represent
depth, height and width, respectively.
The input(X) size and output(Out) size may be different.

Example:   
  Input:
       Input shape: $(N, C_{in}, D_{in}, H_{in}, W_{in})$
       Filter shape: $(C_{in}, C_{out}, D_f, H_f, W_f)$
  Output:
       Output shape: $(N, C_{out}, D_{out}, H_{out}, W_{out})$
  Where
  $$
       D_{out} = (D_{in} - 1) * strides[0] - 2 * paddings[0] + D_f \\
       H_{out} = (H_{in} - 1) * strides[1] - 2 * paddings[1] + H_f \\
       W_{out} = (W_{in} - 1) * strides[2] - 2 * paddings[2] + W_f
  $$
)DOC");
}

void ConvTransposeOpGrad::InferShape(framework::InferShapeContext* ctx) const {
  auto in_dims = ctx->GetInputDim("Input");
  auto filter_dims = ctx->GetInputDim("Filter");
  if (ctx->HasOutput(framework::GradVarName("Input"))) {
    ctx->SetOutputDim(framework::GradVarName("Input"), in_dims);
  }
  if (ctx->HasOutput(framework::GradVarName("Filter"))) {
    ctx->SetOutputDim(framework::GradVarName("Filter"), filter_dims);
  }
}

}  // namespace operators
}  // namespace paddle

namespace ops = paddle::operators;

REGISTER_OP(conv2d_transpose, ops::ConvTransposeOp, ops::Conv2DTransposeOpMaker,
            conv2d_transpose_grad, ops::ConvTransposeOpGrad);

REGISTER_OP_CPU_KERNEL(
    conv2d_transpose,
    ops::GemmConvTransposeKernel<paddle::platform::CPUPlace, float>,
    ops::GemmConvTransposeKernel<paddle::platform::CPUPlace, double>);
REGISTER_OP_CPU_KERNEL(
    conv2d_transpose_grad,
    ops::GemmConvTransposeGradKernel<paddle::platform::CPUPlace, float>,
    ops::GemmConvTransposeGradKernel<paddle::platform::CPUPlace, double>);

REGISTER_OP(conv3d_transpose, ops::ConvTransposeOp, ops::Conv3DTransposeOpMaker,
            conv3d_transpose_grad, ops::ConvTransposeOpGrad);

REGISTER_OP_CPU_KERNEL(
    conv3d_transpose,
    ops::GemmConvTransposeKernel<paddle::platform::CPUPlace, float>,
    ops::GemmConvTransposeKernel<paddle::platform::CPUPlace, double>);
REGISTER_OP_CPU_KERNEL(
    conv3d_transpose_grad,
    ops::GemmConvTransposeGradKernel<paddle::platform::CPUPlace, float>,
    ops::GemmConvTransposeGradKernel<paddle::platform::CPUPlace, double>);
