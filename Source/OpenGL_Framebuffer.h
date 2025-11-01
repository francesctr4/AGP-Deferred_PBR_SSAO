#ifndef _OPENGL_FRAMEBUFFER_H_
#define _OPENGL_FRAMEBUFFER_H_

#include "platform.h"
#include <glad/glad.h>
#include <vector>
#include <utility>

class Framebuffer
{
public:

    Framebuffer();
    ~Framebuffer();

    bool Create(u8 colorAttachmentsCount, glm::vec2 displaySize);
	void Clear();

    GLuint GetFramebufferHandle() const;
    GLuint GetColorAttachment(u8 index) const;
    GLuint GetDepthAttachment() const;
    u8 GetColorAttachmentCount() const;

private:

    GLuint mFBO;
    std::vector<std::pair<GLenum, GLuint>> mColorAttachments;
    GLuint mDepthAttachment;
    glm::vec2 mDisplaySize;

};

#endif // _OPENGL_FRAMEBUFFER_H_