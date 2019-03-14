#pragma once
#include "volk.h"

class DescriptorSetLayout {
  public:
	DescriptorSetLayout();
	~DescriptorSetLayout();
	void InitFromSpirV(const uint32_t* buffer, uint32_t bufferLength);
  private:

};