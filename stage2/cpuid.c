/* cpuid.c: Interface to the x86 CPUID instruction
 *
 * Copyright 2015, 2016 Vincent Damewood
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cpuid.h"
#include "uio.h"

char vendor[13];

char *cpuidVendor()
{
	if (!vendor[0])
	{
		asm ("xor %%eax, %%eax; cpuid; movl %%ebx, %0; movl %%edx, %1; movl %%ecx, %2;"
			: : "m"(vendor[0]), "m"(vendor[4]), "m"(vendor[8])
			: "eax", "ecx", "edx", "ebx");
	}
	return vendor;
}

void cpuidShowVendor(void)
{
	uioPrint(cpuidVendor());
	uioPrintChar('\n');
}
