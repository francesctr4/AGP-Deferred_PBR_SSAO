#include "OpenGL_Framebuffer.h"

Framebuffer::Framebuffer()
{
	mFBO = 0;
	mDepthAttachment = 0;
    mDisplaySize = glm::zero<glm::vec2>();
}

Framebuffer::~Framebuffer()
{
    Clear();
}

bool Framebuffer::Create(u8 colorAttachmentsCount, glm::vec2 displaySize)
{
    Clear();

    mDisplaySize = displaySize;

    // Check Maximum Color Attachments
    GLint maxAttachments;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttachments);
    if (colorAttachmentsCount > maxAttachments) 
    {
        ELOG("Requested %d color attachments, but only %d are supported", colorAttachmentsCount, maxAttachments);
        return false;
    }

    // Create Color Attachments
    for (u8 i = 0; i < colorAttachmentsCount; ++i)
    {
        GLuint colorAttachment;
        glGenTextures(1, &colorAttachment);
        glBindTexture(GL_TEXTURE_2D, colorAttachment);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
            static_cast<GLsizei>(displaySize.x),
            static_cast<GLsizei>(displaySize.y),
            0, GL_RGBA, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_2D, 0);

        mColorAttachments.emplace_back(GL_COLOR_ATTACHMENT0 + i, colorAttachment);
    }

    // Create Depth Attachment
    glGenTextures(1, &mDepthAttachment);
    glBindTexture(GL_TEXTURE_2D, mDepthAttachment);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 
        static_cast<GLsizei>(displaySize.x), 
        static_cast<GLsizei>(displaySize.y), 
        0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    // Create FBO
    glGenFramebuffers(1, &mFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

    // Attach Color Textures
    for (const auto& [attachment, handle] : mColorAttachments) 
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, handle, 0);
    }

    // Attach Depth Texture
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mDepthAttachment, 0);

    // Set Draw Buffers
    if (!mColorAttachments.empty()) 
    {
        std::vector<GLenum> buffers;
        for (const auto& [attachment, _] : mColorAttachments)
        {
            buffers.push_back(attachment);
        }
        glDrawBuffers(static_cast<GLsizei>(buffers.size()), buffers.data());
    }
    else 
    {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    // Check Framebuffer Completeness
    const GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE)
    {
        switch (framebufferStatus)
        {
            case GL_FRAMEBUFFER_UNDEFINED:                      ELOG("GL_FRAMEBUFFER_UNDEFINED");                       break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:          ELOG("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");           break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:  ELOG("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");   break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:         ELOG("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER");          break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:         ELOG("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER");          break;
            case GL_FRAMEBUFFER_UNSUPPORTED:                    ELOG("GL_FRAMEBUFFER_UNSUPPORTED");                     break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:         ELOG("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE");          break;
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:       ELOG("GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS");        break;

            default:                                            ELOG("Unknown framebuffer status error");
        }

        Clear();
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

void Framebuffer::Clear() 
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    for (auto& [attachment, handle] : mColorAttachments)
    {
        glDeleteTextures(1, &handle);
    }
    mColorAttachments.clear();

    if (mDepthAttachment != 0)
    {
        glDeleteTextures(1, &mDepthAttachment);
        mDepthAttachment = 0;
    }

    if (mFBO != 0)
    {
        glDeleteFramebuffers(1, &mFBO);
        mFBO = 0;
    }
}

GLuint Framebuffer::GetFramebufferHandle() const
{
    return mFBO;
}

GLuint Framebuffer::GetColorAttachment(u8 index) const
{
    if (index >= 0 && index < mColorAttachments.size())
    {
        return mColorAttachments[index].second;
    }

    return 0;
}

GLuint Framebuffer::GetDepthAttachment() const
{
    return mDepthAttachment;
}

u8 Framebuffer::GetColorAttachmentCount() const
{
    return static_cast<u8>(mColorAttachments.size());
}
