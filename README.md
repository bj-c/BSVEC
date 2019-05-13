# Bi-sequential Video Error Concealment Method Using Adaptive Homography-based Registration (BSVEC)

[TCSVT 2019 Paper](https://ieeexplore.ieee.org/document/8682097) | [Demo](http://ivp.konkuk.ac.kr/bsvec) | [DataSet]

This software is the implementation for the following publication. B. Chung and C. Yim,"Bi-sequential Video Error Concealment Method Using Adaptive Homography-based Registration," IEEE Transactions on Circuits and Systems for Video Technology, 2019.

## Summary

This paper proposes a novel video error concealment method for the reconstruction of all unknown regions in frames. 

The proposed method first performs temporal error concealment (TEC) sequentially in forward direction with the propagation of source information for the unknown regions of frames in group-of-frames (GOF). 

Then it performs TEC sequentially in backward direction for the remaining unknown regions in GOF. 

For the reconstruction of unknown regions in each target frame in forward and backward TEC stages, we propose an adaptive homography-based registration of the reference frame. 

The proposed method selects adaptively global or local homography-based registration for the reconstruction of each unknown region. 

After the backward direction TEC is performed in each frame, spatial error concealment is applied to reconstruct completely the remaining unknown regions with relatively small sizes. 

Experimental results show that the proposed method gives significantly improved objective and subjective video quality compared with the traditional reconstruction methods. 

The proposed method also gives comparably good reconstruction results for large size unknown regions and is feasible for practical real-time applications.
