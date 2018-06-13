#include <cassert>
#include <algorithm>
#include <cstring>

#include "core.h"


int Core::op_init(void)
{
	/*set up memories*/
	//create weight & bias mem
	
	//网络为四层结构，前两层为卷积层，后两层为全连接层

	//卷积层kernel值的四个维度分别对应x坐标、y坐标、输入通道、输出通道
	_memhub.addMemory("model/conv2d/kernel", 5, 5, 1, 32);
	//卷积层bias值的一个维度对应输出通道
	_memhub.addMemory("model/conv2d/bias", 32);

	_memhub.addMemory("model/conv2d_1/kernel", 5, 5, 32, 64);
	_memhub.addMemory("model/conv2d_1/bias", 64);

	//全连接层kernel值的两个维度分别对应输入坐标和输出坐标
	_memhub.addMemory("model/dense/kernel", 3136, 512);
	//全连接层bias值得一个维度对应输出坐标
	_memhub.addMemory("model/dense/bias", 512);

	_memhub.addMemory("model/dense_1/kernel", 512, 10);
	_memhub.addMemory("model/dense_1/bias", 10);

	//load weight & bias data
	{
		std::ifstream wfile;
		wfile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		wfile.open("data/ModelData", std::ios::in | std::ios::binary);

		Memory *mem = &_memhub.getMemory("model/conv2d/kernel");
		wfile.read(reinterpret_cast<char*>(mem->getPtr()), mem->getMemSizeByByte());
		mem = &_memhub.getMemory("model/conv2d/bias");
		wfile.read(reinterpret_cast<char*>(mem->getPtr()), mem->getMemSizeByByte());

		mem = &_memhub.getMemory("model/conv2d_1/kernel");
		wfile.read(reinterpret_cast<char*>(mem->getPtr()), mem->getMemSizeByByte());
		mem = &_memhub.getMemory("model/conv2d_1/bias");
		wfile.read(reinterpret_cast<char*>(mem->getPtr()), mem->getMemSizeByByte());

		mem = &_memhub.getMemory("model/dense/kernel");
		wfile.read(reinterpret_cast<char*>(mem->getPtr()), mem->getMemSizeByByte());
		mem = &_memhub.getMemory("model/dense/bias");
		wfile.read(reinterpret_cast<char*>(mem->getPtr()), mem->getMemSizeByByte());

		mem = &_memhub.getMemory("model/dense_1/kernel");
		wfile.read(reinterpret_cast<char*>(mem->getPtr()), mem->getMemSizeByByte());
		mem = &_memhub.getMemory("model/dense_1/bias");
		wfile.read(reinterpret_cast<char*>(mem->getPtr()), mem->getMemSizeByByte());
	}
	//create temp data memory
	//这一部分为存储中间结果的存储器
	//其中2维的存储器，2个维度分别对应x坐标和y坐标
	//3维的存储器，3各维度分别对应x坐标、y坐标、通道
	_memhub.addMemory("data/padding/in", 28, 28);
	_memhub.addMemory("data/padding/out|conv2d/in", 32, 32);
	_memhub.addMemory("data/conv2d/out|pool/in", 28, 28, 32);
	_memhub.addMemory("data/pool/out|padding_1/in", 14, 14, 32);
	_memhub.addMemory("data/padding_1/out|conv2d_1/in", 18, 18, 32);
	_memhub.addMemory("data/conv2d_1/out|pool_1/in", 14, 14, 64);
	_memhub.addMemory("data/pool_1/out|dense/in", 7, 7, 64);
	_memhub.addMemory("data/dense/out|dense_1/in", 512);
	_memhub.addMemory("data/dense_1/out", 10);

	/*set up input file*/
	_ifile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	_ifile.open("data/t10k-images.idx3-ubyte", std::ios::in | std::ios::binary);

	//read mnist data header
	char buffer[16];
	_ifile.read(buffer, 16);
	int32_t *p = reinterpret_cast<int32_t*>(buffer);
	if (
		*p++ != SWAP32(2051) ||
		*p++ != SWAP32(10000) ||
		*p++ != SWAP32(28) ||
		*p != SWAP32(28)
		)
	{
		throw simulator_error("Mnist Input: File Format Error");
	}

	return 1000;
}

int Core::op_input(std::string memName)
{
	uint8_t buffer[28 * 28];
	_ifile.read(reinterpret_cast<char*>(buffer), 28 * 28);

	float *ptr = _memhub.getMemory(memName).getPtr();
	for (int x = 0; x < 28; ++x)
	{
		for (int y = 0; y < 28; y++)
		{
			ptr[x * 28 + y] = float(buffer[x * 28 + y]) / 255.f - 0.5f;
		}
	}

	return 100;
}

int Core::op_output(std::string memName)
{
	float *ptr = _memhub.getMemory(memName).getPtr();
	outputIndex = std::max_element(ptr, ptr + 10) - ptr;
	return 10;
}

int Core::op_conv(
	std::string inMemName, 
	std::string outMemName, 
	std::string weightMemName,
	std::string biasMemName,
	int inChannelNum, 
	Range outChannelRange,
	Range outXRange, 
	Range outYRange, 
	int filterSize, 
	int stride
)
{
	/* 请补充完成该函数的代码
	 * 函数的各个输入参数的含义如下：
	 *	inMemName,outMemName,weightMemName,biasMemName: 对应的输入输出值和weight、bias值所在的存储器
	 *	inChannelNum: 输入值的总通道数
	 *	outChannelRange,outXRange,outYRange: 表明本次函数调用仅计算这三个参数指定的输出坐标范围
	 *	filterSize: 卷积核大小，filterSize=5即表示卷积核为5x5
	 *	stride: 卷积步长
	 */

	Memory &in = _memhub.getMemory(inMemName);
	Memory &out = _memhub.getMemory(outMemName);
	Memory &weight = _memhub.getMemory(weightMemName);
	Memory &bias = _memhub.getMemory(biasMemName);

	/***********************************************
	 * 请在此处补充代码
	 * hint: 不要忘记在输出前执行ReLU操作
	 ***********************************************/


	//这里返回了执行卷积操作所需的周期数
	//我们假定使用16x16的PE阵列，可以同时计算256个乘累加，所以最后除以了256
	return (outXRange.end - outXRange.begin)*
		(outYRange.end - outYRange.begin) *
		(outChannelRange.end - outChannelRange.begin)*
		filterSize*filterSize*
		inChannelNum / 256;
}

int Core::op_fc(
	std::string inMemName, 
	std::string outMemName, 
	std::string weightMemName,
	std::string biasMemName,
	int inDataLength, 
	Range outRange
)
{
	/* 请补充完成该函数的代码
	* 函数的各个输入参数的含义如下：
	*	inMemName,outMemName,weightMemName,biasMemName: 对应的输入输出值和weight、bias值所在的存储器
	*	inDataLength: 输入值的长度
	*	outRange: 表明本次函数调用仅计算该参数指定的输出坐标范围
	*/

	Memory &in = _memhub.getMemory(inMemName);
	Memory &out = _memhub.getMemory(outMemName);
	Memory &weight = _memhub.getMemory(weightMemName);
	Memory &bias = _memhub.getMemory(biasMemName);

	/***********************************************
	* 请在此处补充代码
	* hint: 从卷积层过渡到全连接层，需要将数据reshape压平为一维
	*		可以简单地通过 'in.getPtr()[index]' 这种方式来访问输入数据
	*		（而非卷积层中应当使用的 '*in.getPtr(x, y, channel)' ）
	*		来达到压平的效果
	* hint2:同样不要忘记在输出前执行ReLU操作
	***********************************************/

	//这里返回了执行卷积操作所需的周期数
	//我们假定使用16x16的PE阵列，可以同时计算256个乘累加，所以最后除以了256
	return (outRange.end - outRange.begin)*inDataLength / 256;
}

int Core::op_pool(
	std::string inMemName, 
	std::string outMemName, 
	Range outChannelRange, 
	Range outXRange,
	Range outYRange, 
	int filterSize, 
	int stride
)
{
	Memory &in = _memhub.getMemory(inMemName);
	Memory &out = _memhub.getMemory(outMemName);

	for (int outX = outXRange.begin; outX < outXRange.end; outX++)
	{
		for (int outY = outYRange.begin; outY < outYRange.end; outY++)
		{
			for (int z = outChannelRange.begin; z < outChannelRange.end; z++)
			{
				float max = std::numeric_limits<float>::min();
				for (int inX = outX * stride; inX < outX * stride + filterSize; inX++)
				{
					for (int inY = outY * stride; inY < outY * stride + filterSize; inY++)
					{
						max = std::max(max, *in.getPtr(inX, inY, z));
					}
				}
				*out.getPtr(outX, outY, z) = max;
			}
		}
	}

	return (outXRange.end - outXRange.begin) *
		(outYRange.end - outYRange.begin) *
		(outChannelRange.end - outChannelRange.begin) *
		stride * stride;
}

int Core::op_padding(
	std::string inMemName, 
	std::string outMemName, 
	int padLength, 
	int inXLen, 
	int inYLen, 
	int inZLen,
	int inWLen
)
{
	Memory &in = _memhub.getMemory(inMemName);
	Memory &out = _memhub.getMemory(outMemName);

	//first make output memory empty
	std::memset(out.getPtr(), 0, out.getMemSizeByByte());

	for (int x = 0; x < inXLen; x++)
	{
		for (int y = 0; y < inYLen;	y++)
		{
			for (int z = 0; z < inZLen; z++)
			{
				for (int w = 0; w < inWLen; w++)
				{
					*out.getPtr(x + padLength, y + padLength, z, w) =
						*in.getPtr(x, y, z, w);
				}
			}
		}
	}

	return 1;
}
