
%module libpyper




%section "opengl"
%include opengl.i


%section "math"
%include linmath.i


%section "geometric primitives"
%include geomath.i


%section "nodes"
%include node.i



%include group.i
%include scs.i
%include dcs.i
%include switch.i
%include lightsource.i
%include scene.i



%section "geometry"
%include geostate.i
%include geoset.i
%include geode.i
%include billboard.i
%include text.i
%include material.i


%section "pipes & channels"
%include pipe.i
%include pipewindow.i
%include channel.i


%section "utils"
%include pr.i

%include pfType.i
%include string.i
# %include font.i
%include pf.i



%include pfdu.i
%include pfutil.i

%include pfdb.i


