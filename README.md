# Bi-sequential Video Error Concealment Method Using Adaptive Homography-based Registration (BSVEC)

[TCSVT 2019 Paper](https://ieeexplore.ieee.org/document/8682097) | [Demo](http://ivp.konkuk.ac.kr/bsvec) | [DataSet](http://ivp.konkuk.ac.kr/files/DataSet.zip)

This software is the implementation for the following publication. B. Chung and C. Yim,"Bi-sequential Video Error Concealment Method Using Adaptive Homography-based Registration," IEEE Transactions on Circuits and Systems for Video Technology, 2019.

Other error concealment and inpainting methods are also implemented in the software.

2D method
- Bilinear interpolation (BI) method
- diffusion-based inpainting (DI) method

3D method
- Bi-sequential video error concealment (BSVEC)
- Temporal replacement (TR) method

## Summary

This paper proposes a novel video error concealment method for the reconstruction of all unknown regions in frames. 

The proposed method first performs temporal error concealment (TEC) sequentially in forward direction with the propagation of source information for the unknown regions of frames in group-of-frames (GOF). 

Then it performs TEC sequentially in backward direction for the remaining unknown regions in GOF. 

For the reconstruction of unknown regions in each target frame in forward and backward TEC stages, we propose an adaptive homography-based registration of the reference frame. 

The proposed method selects adaptively global or local homography-based registration for the reconstruction of each unknown region. 

After the backward direction TEC is performed in each frame, spatial error concealment is applied to reconstruct completely the remaining unknown regions with relatively small sizes. 

Experimental results show that the proposed method gives significantly improved objective and subjective video quality compared with the traditional reconstruction methods. 

The proposed method also gives comparably good reconstruction results for large size unknown regions and is feasible for practical real-time applications.

## TCSVT 2019 Paper
[TCSVT 2019 Paper](https://ieeexplore.ieee.org/document/8682097)

## Project Demo
[Demo](http://ivp.konkuk.ac.kr/bsvec)

- These video files are the simulation results of the proposed method and the previous state-of-the-art methods for the comparison of subjective video quality.

- The manuscript for the proposed method is entitled “Bi-sequential Video Error Concealment Method Using Adaptive Homography-based Registration,”  

- The previous methods for comparison are as follows.

- Newson method:  A. Newson, A. Almansa, M. Fradet, Y. Gousseau, and P. Perez, “Video inpainting of complex scenes,” 

SIAM J. Imag. Sci., vol. 7, no. 4, pp.1993-2019, Oct. 2014.

- Huang method: J.-B. Huang, S. B. Kang, N. Ahuja, and J. Kopf, “Temporally coherent completion of dynamic video,” 

ACM Trans. Graph., vol. 35, no. 6, Art. 196, Nov. 2016.

## DataSet
[DataSet](http://ivp.konkuk.ac.kr/files/DataSet.zip)

1. Parking Lot video (CTU type, Loss rate=5%)
2. Parking Lot video (CTU type, Loss rate=10%)
3. Parking Lot video (CTU type, Loss rate=20%)
4. Hall video (CTU type, Loss rate=5%)
5. Hall video (CTU type, Loss rate=10%)
6. Hall video (CTU type, Loss rate=20%)
7. Hall video (Slice type, Loss rate=15%)
8. Hall video (Slice type, Loss rate=25%)
9. Building video (CTU type, Loss rate=5%)
10. Building video (CTU type, Loss rate=10%)
11. Building video (CTU type, Loss rate=20%)
12. Building video (Slice type, Loss rate=15%)
13. Building video (Slice type, Loss rate=25%)
14. Bamboo video (CTU type, Loss rate=20%)
15. Sleeping video (CTU type, Loss rate=20%)
16. Cave video (CTU type, Loss rate=20%)
