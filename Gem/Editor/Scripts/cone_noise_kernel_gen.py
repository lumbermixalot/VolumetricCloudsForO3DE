# This is a very specialized script
# only used to generate random vectors for the
# NOISE_KERNEL array in Cloudscape.azsl

import argparse
import random
from datetime import datetime
import math


def GenerateShaderText(arrayName: str, vectorList: list) -> str:
    tabs = "    "
    text = f"{tabs}static const float3 {arrayName}[{len(vectorList)}] = {{\n"
    for xyz in vectorList:
        text += f"{tabs}{tabs}float3({xyz[0]:.8f}, {xyz[1]:.8f}, {xyz[2]:.8f}),\n"
    text += f"{tabs}}};\n"
    return text
                                                                    

def GenerateVectorList(vectorCount: int, seed: float) -> list:
    random.seed(seed)
    retList = []
    for idx in range(vectorCount):
        x = (random.random() * 2.0) - 1.0
        y = (random.random() * 2.0) - 1.0
        z = (random.random() * 2.0) - 1.0
        norm = math.sqrt(x*x + y*y + z*z)
        retList.append((x/norm, y/norm, z/norm))
    return retList

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Generates NOISE_KERNEL table in Cloudscape.azsl')
    parser.add_argument('--step_count', '--s', default=6, type=int, action='store',
                    help='How many vectors in the NOISE_KERNEL array.')
    args = parser.parse_args()
    vectorCount = max(1, args.step_count)
    vectorList = GenerateVectorList(vectorCount, datetime.now().timestamp())
    shaderText = GenerateShaderText("NOISE_KERNEL", vectorList)
    print(shaderText)