#include <cassert>
#include <algorithm>
#include <cstring>

#include "core.h"


int Core::op_init(void)
{
	/*set up memories*/
	//create weight & bias mem
	
	//����Ϊ�Ĳ�ṹ��ǰ����Ϊ����㣬������Ϊȫ���Ӳ�

	//�����kernelֵ���ĸ�ά�ȷֱ��Ӧx���ꡢy���ꡢ����ͨ�������ͨ��
	_memhub.addMemory("model/conv2d/kernel", 5, 5, 1, 32);
	//�����biasֵ��һ��ά�ȶ�Ӧ���ͨ��
	_memhub.addMemory("model/conv2d/bias", 32);

	_memhub.addMemory("model/conv2d_1/kernel", 5, 5, 32, 64);
	_memhub.addMemory("model/conv2d_1/bias", 64);

	//ȫ���Ӳ�kernelֵ������ά�ȷֱ��Ӧ����������������
	_memhub.addMemory("model/dense/kernel", 3136, 512);
	//ȫ���Ӳ�biasֵ��һ��ά�ȶ�Ӧ�������
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
	//��һ����Ϊ�洢�м����Ĵ洢��
	//����2ά�Ĵ洢����2��ά�ȷֱ��Ӧx�����y����
	//3ά�Ĵ洢����3��ά�ȷֱ��Ӧx���ꡢy���ꡢͨ��
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
	/* �벹����ɸú����Ĵ���
	 * �����ĸ�����������ĺ������£�
	 *	inMemName,outMemName,weightMemName,biasMemName: ��Ӧ���������ֵ��weight��biasֵ���ڵĴ洢��
	 *	inChannelNum: ����ֵ����ͨ����
	 *	outChannelRange,outXRange,outYRange: �������κ������ý���������������ָ����������귶Χ
	 *	filterSize: ����˴�С��filterSize=5����ʾ�����Ϊ5x5
	 *	stride: �������
	 */

	Memory &in = _memhub.getMemory(inMemName);
	Memory &out = _memhub.getMemory(outMemName);
	Memory &weight = _memhub.getMemory(weightMemName);
	Memory &bias = _memhub.getMemory(biasMemName);

	/***********************************************
	 * ���ڴ˴��������
	 * hint: ��Ҫ���������ǰִ��ReLU����
	 ***********************************************/


	//���ﷵ����ִ�о�����������������
	//���Ǽٶ�ʹ��16x16��PE���У�����ͬʱ����256�����ۼӣ�������������256
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
	/* �벹����ɸú����Ĵ���
	* �����ĸ�����������ĺ������£�
	*	inMemName,outMemName,weightMemName,biasMemName: ��Ӧ���������ֵ��weight��biasֵ���ڵĴ洢��
	*	inDataLength: ����ֵ�ĳ���
	*	outRange: �������κ������ý�����ò���ָ����������귶Χ
	*/

	Memory &in = _memhub.getMemory(inMemName);
	Memory &out = _memhub.getMemory(outMemName);
	Memory &weight = _memhub.getMemory(weightMemName);
	Memory &bias = _memhub.getMemory(biasMemName);

	/***********************************************
	* ���ڴ˴��������
	* hint: �Ӿ������ɵ�ȫ���Ӳ㣬��Ҫ������reshapeѹƽΪһά
	*		���Լ򵥵�ͨ�� 'in.getPtr()[index]' ���ַ�ʽ��������������
	*		�����Ǿ������Ӧ��ʹ�õ� '*in.getPtr(x, y, channel)' ��
	*		���ﵽѹƽ��Ч��
	* hint2:ͬ����Ҫ���������ǰִ��ReLU����
	***********************************************/

	//���ﷵ����ִ�о�����������������
	//���Ǽٶ�ʹ��16x16��PE���У�����ͬʱ����256�����ۼӣ�������������256
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
