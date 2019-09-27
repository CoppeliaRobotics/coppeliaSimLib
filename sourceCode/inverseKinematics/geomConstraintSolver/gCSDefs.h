
#ifndef _GLOBAL_GCS_
#define _GLOBAL_GCS_

#define GCSIK_DIVISION_FACTOR 100.0f // needed for better stability of resolutions!

// IK Object types
enum {  IK_JOINT_TYPE=0,
        IK_DUMMY_TYPE,
        IK_MESH_TYPE
};

// IK Graph Node types
enum {  IK_GRAPH_JOINT_TYPE=0,
        IK_GRAPH_OBJECT_TYPE
};

// IK Graph Object types
enum {  IK_GRAPH_LINK_OBJECT_TYPE=0,
        IK_GRAPH_TIP_OBJECT_TYPE,
        IK_GRAPH_PASSIVE_OBJECT_TYPE,
        IK_GRAPH_MESH_OBJECT_TYPE
};

// IK Graph Joint types
enum {  IK_GRAPH_REVOLUTE_JOINT_TYPE=0,
        IK_GRAPH_PRISMATIC_JOINT_TYPE,
        IK_GRAPH_SPHERICAL_JOINT_TYPE,
        IK_GRAPH_SCREW_JOINT_TYPE
};

// Ik constraints.
enum {
        IK_X_CONSTRAINT=1,
        IK_Y_CONSTRAINT=2,
        IK_Z_CONSTRAINT=4,
        IK_ALPHA_BETA_CONSTRAINT=8,
        IK_GAMMA_CONSTRAINT=16,
// FREE
        IK_AVOIDANCE_CONSTRAINT=64
};

#endif // !defined(_GLOBAL_GCS_)
