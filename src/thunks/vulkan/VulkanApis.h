#ifndef VULKANAPIS_H
#define VULKANAPIS_H

#include <lorelib_common/manifest-predef.h>

#define VK_USE_PLATFORM_XLIB_KHR
#define VK_USE_PLATFORM_XCB_KHR
#define VK_USE_PLATFORM_WAYLAND_KHR

#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LORELIB_VULKAN_API_FOREACH(F)                                                              \
    F(vkCreateInstance)                                                                            \
    F(vkDestroyInstance)                                                                           \
    F(vkEnumeratePhysicalDevices)                                                                  \
    F(vkGetPhysicalDeviceFeatures)                                                                 \
    F(vkGetPhysicalDeviceFormatProperties)                                                         \
    F(vkGetPhysicalDeviceImageFormatProperties)                                                    \
    F(vkGetPhysicalDeviceProperties)                                                               \
    F(vkGetPhysicalDeviceQueueFamilyProperties)                                                    \
    F(vkGetPhysicalDeviceMemoryProperties)                                                         \
    F(vkGetInstanceProcAddr)                                                                       \
    F(vkGetDeviceProcAddr)                                                                         \
    F(vkCreateDevice)                                                                              \
    F(vkDestroyDevice)                                                                             \
    F(vkEnumerateInstanceExtensionProperties)                                                      \
    F(vkEnumerateDeviceExtensionProperties)                                                        \
    F(vkEnumerateInstanceLayerProperties)                                                          \
    F(vkEnumerateDeviceLayerProperties)                                                            \
    F(vkGetDeviceQueue)                                                                            \
    F(vkQueueSubmit)                                                                               \
    F(vkQueueWaitIdle)                                                                             \
    F(vkDeviceWaitIdle)                                                                            \
    F(vkAllocateMemory)                                                                            \
    F(vkFreeMemory)                                                                                \
    F(vkMapMemory)                                                                                 \
    F(vkUnmapMemory)                                                                               \
    F(vkFlushMappedMemoryRanges)                                                                   \
    F(vkInvalidateMappedMemoryRanges)                                                              \
    F(vkGetDeviceMemoryCommitment)                                                                 \
    F(vkBindBufferMemory)                                                                          \
    F(vkBindImageMemory)                                                                           \
    F(vkGetBufferMemoryRequirements)                                                               \
    F(vkGetImageMemoryRequirements)                                                                \
    F(vkGetImageSparseMemoryRequirements)                                                          \
    F(vkGetPhysicalDeviceSparseImageFormatProperties)                                              \
    F(vkQueueBindSparse)                                                                           \
    F(vkCreateFence)                                                                               \
    F(vkDestroyFence)                                                                              \
    F(vkResetFences)                                                                               \
    F(vkGetFenceStatus)                                                                            \
    F(vkWaitForFences)                                                                             \
    F(vkCreateSemaphore)                                                                           \
    F(vkDestroySemaphore)                                                                          \
    F(vkCreateEvent)                                                                               \
    F(vkDestroyEvent)                                                                              \
    F(vkGetEventStatus)                                                                            \
    F(vkSetEvent)                                                                                  \
    F(vkResetEvent)                                                                                \
    F(vkCreateQueryPool)                                                                           \
    F(vkDestroyQueryPool)                                                                          \
    F(vkGetQueryPoolResults)                                                                       \
    F(vkCreateBuffer)                                                                              \
    F(vkDestroyBuffer)                                                                             \
    F(vkCreateBufferView)                                                                          \
    F(vkDestroyBufferView)                                                                         \
    F(vkCreateImage)                                                                               \
    F(vkDestroyImage)                                                                              \
    F(vkGetImageSubresourceLayout)                                                                 \
    F(vkCreateImageView)                                                                           \
    F(vkDestroyImageView)                                                                          \
    F(vkCreateShaderModule)                                                                        \
    F(vkDestroyShaderModule)                                                                       \
    F(vkCreatePipelineCache)                                                                       \
    F(vkDestroyPipelineCache)                                                                      \
    F(vkGetPipelineCacheData)                                                                      \
    F(vkMergePipelineCaches)                                                                       \
    F(vkCreateGraphicsPipelines)                                                                   \
    F(vkCreateComputePipelines)                                                                    \
    F(vkDestroyPipeline)                                                                           \
    F(vkCreatePipelineLayout)                                                                      \
    F(vkDestroyPipelineLayout)                                                                     \
    F(vkCreateSampler)                                                                             \
    F(vkDestroySampler)                                                                            \
    F(vkCreateDescriptorSetLayout)                                                                 \
    F(vkDestroyDescriptorSetLayout)                                                                \
    F(vkCreateDescriptorPool)                                                                      \
    F(vkDestroyDescriptorPool)                                                                     \
    F(vkResetDescriptorPool)                                                                       \
    F(vkAllocateDescriptorSets)                                                                    \
    F(vkFreeDescriptorSets)                                                                        \
    F(vkUpdateDescriptorSets)                                                                      \
    F(vkCreateFramebuffer)                                                                         \
    F(vkDestroyFramebuffer)                                                                        \
    F(vkCreateRenderPass)                                                                          \
    F(vkDestroyRenderPass)                                                                         \
    F(vkGetRenderAreaGranularity)                                                                  \
    F(vkCreateCommandPool)                                                                         \
    F(vkDestroyCommandPool)                                                                        \
    F(vkResetCommandPool)                                                                          \
    F(vkAllocateCommandBuffers)                                                                    \
    F(vkFreeCommandBuffers)                                                                        \
    F(vkBeginCommandBuffer)                                                                        \
    F(vkEndCommandBuffer)                                                                          \
    F(vkResetCommandBuffer)                                                                        \
    F(vkCmdBindPipeline)                                                                           \
    F(vkCmdSetViewport)                                                                            \
    F(vkCmdSetScissor)                                                                             \
    F(vkCmdSetLineWidth)                                                                           \
    F(vkCmdSetDepthBias)                                                                           \
    F(vkCmdSetBlendConstants)                                                                      \
    F(vkCmdSetDepthBounds)                                                                         \
    F(vkCmdSetStencilCompareMask)                                                                  \
    F(vkCmdSetStencilWriteMask)                                                                    \
    F(vkCmdSetStencilReference)                                                                    \
    F(vkCmdBindDescriptorSets)                                                                     \
    F(vkCmdBindIndexBuffer)                                                                        \
    F(vkCmdBindVertexBuffers)                                                                      \
    F(vkCmdDraw)                                                                                   \
    F(vkCmdDrawIndexed)                                                                            \
    F(vkCmdDrawIndirect)                                                                           \
    F(vkCmdDrawIndexedIndirect)                                                                    \
    F(vkCmdDispatch)                                                                               \
    F(vkCmdDispatchIndirect)                                                                       \
    F(vkCmdCopyBuffer)                                                                             \
    F(vkCmdCopyImage)                                                                              \
    F(vkCmdBlitImage)                                                                              \
    F(vkCmdCopyBufferToImage)                                                                      \
    F(vkCmdCopyImageToBuffer)                                                                      \
    F(vkCmdUpdateBuffer)                                                                           \
    F(vkCmdFillBuffer)                                                                             \
    F(vkCmdClearColorImage)                                                                        \
    F(vkCmdClearDepthStencilImage)                                                                 \
    F(vkCmdClearAttachments)                                                                       \
    F(vkCmdResolveImage)                                                                           \
    F(vkCmdSetEvent)                                                                               \
    F(vkCmdResetEvent)                                                                             \
    F(vkCmdWaitEvents)                                                                             \
    F(vkCmdPipelineBarrier)                                                                        \
    F(vkCmdBeginQuery)                                                                             \
    F(vkCmdEndQuery)                                                                               \
    F(vkCmdResetQueryPool)                                                                         \
    F(vkCmdWriteTimestamp)                                                                         \
    F(vkCmdCopyQueryPoolResults)                                                                   \
    F(vkCmdPushConstants)                                                                          \
    F(vkCmdBeginRenderPass)                                                                        \
    F(vkCmdNextSubpass)                                                                            \
    F(vkCmdEndRenderPass)                                                                          \
    F(vkCmdExecuteCommands)                                                                        \
    F(vkEnumerateInstanceVersion)                                                                  \
    F(vkBindBufferMemory2)                                                                         \
    F(vkBindImageMemory2)                                                                          \
    F(vkGetDeviceGroupPeerMemoryFeatures)                                                          \
    F(vkCmdSetDeviceMask)                                                                          \
    F(vkCmdDispatchBase)                                                                           \
    F(vkEnumeratePhysicalDeviceGroups)                                                             \
    F(vkGetImageMemoryRequirements2)                                                               \
    F(vkGetBufferMemoryRequirements2)                                                              \
    F(vkGetImageSparseMemoryRequirements2)                                                         \
    F(vkGetPhysicalDeviceFeatures2)                                                                \
    F(vkGetPhysicalDeviceProperties2)                                                              \
    F(vkGetPhysicalDeviceFormatProperties2)                                                        \
    F(vkGetPhysicalDeviceImageFormatProperties2)                                                   \
    F(vkGetPhysicalDeviceQueueFamilyProperties2)                                                   \
    F(vkGetPhysicalDeviceMemoryProperties2)                                                        \
    F(vkGetPhysicalDeviceSparseImageFormatProperties2)                                             \
    F(vkTrimCommandPool)                                                                           \
    F(vkGetDeviceQueue2)                                                                           \
    F(vkCreateSamplerYcbcrConversion)                                                              \
    F(vkDestroySamplerYcbcrConversion)                                                             \
    F(vkCreateDescriptorUpdateTemplate)                                                            \
    F(vkDestroyDescriptorUpdateTemplate)                                                           \
    F(vkUpdateDescriptorSetWithTemplate)                                                           \
    F(vkGetPhysicalDeviceExternalBufferProperties)                                                 \
    F(vkGetPhysicalDeviceExternalFenceProperties)                                                  \
    F(vkGetPhysicalDeviceExternalSemaphoreProperties)                                              \
    F(vkGetDescriptorSetLayoutSupport)                                                             \
    F(vkCmdDrawIndirectCount)                                                                      \
    F(vkCmdDrawIndexedIndirectCount)                                                               \
    F(vkCreateRenderPass2)                                                                         \
    F(vkCmdBeginRenderPass2)                                                                       \
    F(vkCmdNextSubpass2)                                                                           \
    F(vkCmdEndRenderPass2)                                                                         \
    F(vkResetQueryPool)                                                                            \
    F(vkGetSemaphoreCounterValue)                                                                  \
    F(vkWaitSemaphores)                                                                            \
    F(vkSignalSemaphore)                                                                           \
    F(vkGetBufferDeviceAddress)                                                                    \
    F(vkGetBufferOpaqueCaptureAddress)                                                             \
    F(vkGetDeviceMemoryOpaqueCaptureAddress)                                                       \
    F(vkGetPhysicalDeviceToolProperties)                                                           \
    F(vkCreatePrivateDataSlot)                                                                     \
    F(vkDestroyPrivateDataSlot)                                                                    \
    F(vkSetPrivateData)                                                                            \
    F(vkGetPrivateData)                                                                            \
    F(vkCmdSetEvent2)                                                                              \
    F(vkCmdResetEvent2)                                                                            \
    F(vkCmdWaitEvents2)                                                                            \
    F(vkCmdPipelineBarrier2)                                                                       \
    F(vkCmdWriteTimestamp2)                                                                        \
    F(vkQueueSubmit2)                                                                              \
    F(vkCmdCopyBuffer2)                                                                            \
    F(vkCmdCopyImage2)                                                                             \
    F(vkCmdCopyBufferToImage2)                                                                     \
    F(vkCmdCopyImageToBuffer2)                                                                     \
    F(vkCmdBlitImage2)                                                                             \
    F(vkCmdResolveImage2)                                                                          \
    F(vkCmdBeginRendering)                                                                         \
    F(vkCmdEndRendering)                                                                           \
    F(vkCmdSetCullMode)                                                                            \
    F(vkCmdSetFrontFace)                                                                           \
    F(vkCmdSetPrimitiveTopology)                                                                   \
    F(vkCmdSetViewportWithCount)                                                                   \
    F(vkCmdSetScissorWithCount)                                                                    \
    F(vkCmdBindVertexBuffers2)                                                                     \
    F(vkCmdSetDepthTestEnable)                                                                     \
    F(vkCmdSetDepthWriteEnable)                                                                    \
    F(vkCmdSetDepthCompareOp)                                                                      \
    F(vkCmdSetDepthBoundsTestEnable)                                                               \
    F(vkCmdSetStencilTestEnable)                                                                   \
    F(vkCmdSetStencilOp)                                                                           \
    F(vkCmdSetRasterizerDiscardEnable)                                                             \
    F(vkCmdSetDepthBiasEnable)                                                                     \
    F(vkCmdSetPrimitiveRestartEnable)                                                              \
    F(vkGetDeviceBufferMemoryRequirements)                                                         \
    F(vkGetDeviceImageMemoryRequirements)                                                          \
    F(vkGetDeviceImageSparseMemoryRequirements)                                                    \
    F(vkCmdSetLineStipple)                                                                         \
    F(vkMapMemory2)                                                                                \
    F(vkUnmapMemory2)                                                                              \
    F(vkCmdBindIndexBuffer2)                                                                       \
    F(vkGetRenderingAreaGranularity)                                                               \
    F(vkGetDeviceImageSubresourceLayout)                                                           \
    F(vkGetImageSubresourceLayout2)                                                                \
    F(vkCmdPushDescriptorSet)                                                                      \
    F(vkCmdPushDescriptorSetWithTemplate)                                                          \
    F(vkCmdSetRenderingAttachmentLocations)                                                        \
    F(vkCmdSetRenderingInputAttachmentIndices)                                                     \
    F(vkCmdBindDescriptorSets2)                                                                    \
    F(vkCmdPushConstants2)                                                                         \
    F(vkCmdPushDescriptorSet2)                                                                     \
    F(vkCmdPushDescriptorSetWithTemplate2)                                                         \
    F(vkCopyMemoryToImage)                                                                         \
    F(vkCopyImageToMemory)                                                                         \
    F(vkCopyImageToImage)                                                                          \
    F(vkTransitionImageLayout)                                                                     \
    F(vkDestroySurfaceKHR)                                                                         \
    F(vkGetPhysicalDeviceSurfaceSupportKHR)                                                        \
    F(vkGetPhysicalDeviceSurfaceCapabilitiesKHR)                                                   \
    F(vkGetPhysicalDeviceSurfaceFormatsKHR)                                                        \
    F(vkGetPhysicalDeviceSurfacePresentModesKHR)                                                   \
    F(vkCreateSwapchainKHR)                                                                        \
    F(vkDestroySwapchainKHR)                                                                       \
    F(vkGetSwapchainImagesKHR)                                                                     \
    F(vkAcquireNextImageKHR)                                                                       \
    F(vkQueuePresentKHR)                                                                           \
    F(vkGetDeviceGroupPresentCapabilitiesKHR)                                                      \
    F(vkGetDeviceGroupSurfacePresentModesKHR)                                                      \
    F(vkGetPhysicalDevicePresentRectanglesKHR)                                                     \
    F(vkAcquireNextImage2KHR)                                                                      \
    F(vkGetPhysicalDeviceDisplayPropertiesKHR)                                                     \
    F(vkGetPhysicalDeviceDisplayPlanePropertiesKHR)                                                \
    F(vkGetDisplayPlaneSupportedDisplaysKHR)                                                       \
    F(vkGetDisplayModePropertiesKHR)                                                               \
    F(vkCreateDisplayModeKHR)                                                                      \
    F(vkGetDisplayPlaneCapabilitiesKHR)                                                            \
    F(vkCreateDisplayPlaneSurfaceKHR)                                                              \
    F(vkCreateSharedSwapchainsKHR)                                                                 \
    F(vkGetPhysicalDeviceVideoCapabilitiesKHR)                                                     \
    F(vkGetPhysicalDeviceVideoFormatPropertiesKHR)                                                 \
    F(vkCreateVideoSessionKHR)                                                                     \
    F(vkDestroyVideoSessionKHR)                                                                    \
    F(vkGetVideoSessionMemoryRequirementsKHR)                                                      \
    F(vkBindVideoSessionMemoryKHR)                                                                 \
    F(vkCreateVideoSessionParametersKHR)                                                           \
    F(vkUpdateVideoSessionParametersKHR)                                                           \
    F(vkDestroyVideoSessionParametersKHR)                                                          \
    F(vkCmdBeginVideoCodingKHR)                                                                    \
    F(vkCmdEndVideoCodingKHR)                                                                      \
    F(vkCmdControlVideoCodingKHR)                                                                  \
    F(vkCmdDecodeVideoKHR)                                                                         \
    F(vkCmdBeginRenderingKHR)                                                                      \
    F(vkCmdEndRenderingKHR)                                                                        \
    F(vkGetPhysicalDeviceFeatures2KHR)                                                             \
    F(vkGetPhysicalDeviceProperties2KHR)                                                           \
    F(vkGetPhysicalDeviceFormatProperties2KHR)                                                     \
    F(vkGetPhysicalDeviceImageFormatProperties2KHR)                                                \
    F(vkGetPhysicalDeviceQueueFamilyProperties2KHR)                                                \
    F(vkGetPhysicalDeviceMemoryProperties2KHR)                                                     \
    F(vkGetPhysicalDeviceSparseImageFormatProperties2KHR)                                          \
    F(vkGetDeviceGroupPeerMemoryFeaturesKHR)                                                       \
    F(vkCmdSetDeviceMaskKHR)                                                                       \
    F(vkCmdDispatchBaseKHR)                                                                        \
    F(vkTrimCommandPoolKHR)                                                                        \
    F(vkEnumeratePhysicalDeviceGroupsKHR)                                                          \
    F(vkGetPhysicalDeviceExternalBufferPropertiesKHR)                                              \
    F(vkGetMemoryFdKHR)                                                                            \
    F(vkGetMemoryFdPropertiesKHR)                                                                  \
    F(vkGetPhysicalDeviceExternalSemaphorePropertiesKHR)                                           \
    F(vkImportSemaphoreFdKHR)                                                                      \
    F(vkGetSemaphoreFdKHR)                                                                         \
    F(vkCmdPushDescriptorSetKHR)                                                                   \
    F(vkCmdPushDescriptorSetWithTemplateKHR)                                                       \
    F(vkCreateDescriptorUpdateTemplateKHR)                                                         \
    F(vkDestroyDescriptorUpdateTemplateKHR)                                                        \
    F(vkUpdateDescriptorSetWithTemplateKHR)                                                        \
    F(vkCreateRenderPass2KHR)                                                                      \
    F(vkCmdBeginRenderPass2KHR)                                                                    \
    F(vkCmdNextSubpass2KHR)                                                                        \
    F(vkCmdEndRenderPass2KHR)                                                                      \
    F(vkGetSwapchainStatusKHR)                                                                     \
    F(vkGetPhysicalDeviceExternalFencePropertiesKHR)                                               \
    F(vkImportFenceFdKHR)                                                                          \
    F(vkGetFenceFdKHR)                                                                             \
    F(vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR)                             \
    F(vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR)                                     \
    F(vkAcquireProfilingLockKHR)                                                                   \
    F(vkReleaseProfilingLockKHR)                                                                   \
    F(vkGetPhysicalDeviceSurfaceCapabilities2KHR)                                                  \
    F(vkGetPhysicalDeviceSurfaceFormats2KHR)                                                       \
    F(vkGetPhysicalDeviceDisplayProperties2KHR)                                                    \
    F(vkGetPhysicalDeviceDisplayPlaneProperties2KHR)                                               \
    F(vkGetDisplayModeProperties2KHR)                                                              \
    F(vkGetDisplayPlaneCapabilities2KHR)                                                           \
    F(vkGetImageMemoryRequirements2KHR)                                                            \
    F(vkGetBufferMemoryRequirements2KHR)                                                           \
    F(vkGetImageSparseMemoryRequirements2KHR)                                                      \
    F(vkCreateSamplerYcbcrConversionKHR)                                                           \
    F(vkDestroySamplerYcbcrConversionKHR)                                                          \
    F(vkBindBufferMemory2KHR)                                                                      \
    F(vkBindImageMemory2KHR)                                                                       \
    F(vkGetDescriptorSetLayoutSupportKHR)                                                          \
    F(vkCmdDrawIndirectCountKHR)                                                                   \
    F(vkCmdDrawIndexedIndirectCountKHR)                                                            \
    F(vkGetSemaphoreCounterValueKHR)                                                               \
    F(vkWaitSemaphoresKHR)                                                                         \
    F(vkSignalSemaphoreKHR)                                                                        \
    F(vkGetPhysicalDeviceFragmentShadingRatesKHR)                                                  \
    F(vkCmdSetFragmentShadingRateKHR)                                                              \
    F(vkCmdSetRenderingAttachmentLocationsKHR)                                                     \
    F(vkCmdSetRenderingInputAttachmentIndicesKHR)                                                  \
    F(vkWaitForPresentKHR)                                                                         \
    F(vkGetBufferDeviceAddressKHR)                                                                 \
    F(vkGetBufferOpaqueCaptureAddressKHR)                                                          \
    F(vkGetDeviceMemoryOpaqueCaptureAddressKHR)                                                    \
    F(vkCreateDeferredOperationKHR)                                                                \
    F(vkDestroyDeferredOperationKHR)                                                               \
    F(vkGetDeferredOperationMaxConcurrencyKHR)                                                     \
    F(vkGetDeferredOperationResultKHR)                                                             \
    F(vkDeferredOperationJoinKHR)                                                                  \
    F(vkGetPipelineExecutablePropertiesKHR)                                                        \
    F(vkGetPipelineExecutableStatisticsKHR)                                                        \
    F(vkGetPipelineExecutableInternalRepresentationsKHR)                                           \
    F(vkMapMemory2KHR)                                                                             \
    F(vkUnmapMemory2KHR)                                                                           \
    F(vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR)                                     \
    F(vkGetEncodedVideoSessionParametersKHR)                                                       \
    F(vkCmdEncodeVideoKHR)                                                                         \
    F(vkCmdSetEvent2KHR)                                                                           \
    F(vkCmdResetEvent2KHR)                                                                         \
    F(vkCmdWaitEvents2KHR)                                                                         \
    F(vkCmdPipelineBarrier2KHR)                                                                    \
    F(vkCmdWriteTimestamp2KHR)                                                                     \
    F(vkQueueSubmit2KHR)                                                                           \
    F(vkCmdCopyBuffer2KHR)                                                                         \
    F(vkCmdCopyImage2KHR)                                                                          \
    F(vkCmdCopyBufferToImage2KHR)                                                                  \
    F(vkCmdCopyImageToBuffer2KHR)                                                                  \
    F(vkCmdBlitImage2KHR)                                                                          \
    F(vkCmdResolveImage2KHR)                                                                       \
    F(vkCmdTraceRaysIndirect2KHR)                                                                  \
    F(vkGetDeviceBufferMemoryRequirementsKHR)                                                      \
    F(vkGetDeviceImageMemoryRequirementsKHR)                                                       \
    F(vkGetDeviceImageSparseMemoryRequirementsKHR)                                                 \
    F(vkCmdBindIndexBuffer2KHR)                                                                    \
    F(vkGetRenderingAreaGranularityKHR)                                                            \
    F(vkGetDeviceImageSubresourceLayoutKHR)                                                        \
    F(vkGetImageSubresourceLayout2KHR)                                                             \
    F(vkWaitForPresent2KHR)                                                                        \
    F(vkCreatePipelineBinariesKHR)                                                                 \
    F(vkDestroyPipelineBinaryKHR)                                                                  \
    F(vkGetPipelineKeyKHR)                                                                         \
    F(vkGetPipelineBinaryDataKHR)                                                                  \
    F(vkReleaseCapturedPipelineDataKHR)                                                            \
    F(vkReleaseSwapchainImagesKHR)                                                                 \
    F(vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR)                                           \
    F(vkCmdSetLineStippleKHR)                                                                      \
    F(vkGetPhysicalDeviceCalibrateableTimeDomainsKHR)                                              \
    F(vkGetCalibratedTimestampsKHR)                                                                \
    F(vkCmdBindDescriptorSets2KHR)                                                                 \
    F(vkCmdPushConstants2KHR)                                                                      \
    F(vkCmdPushDescriptorSet2KHR)                                                                  \
    F(vkCmdPushDescriptorSetWithTemplate2KHR)                                                      \
    F(vkCmdSetDescriptorBufferOffsets2EXT)                                                         \
    F(vkCmdBindDescriptorBufferEmbeddedSamplers2EXT)                                               \
    F(vkCreateDebugReportCallbackEXT)                                                              \
    F(vkDestroyDebugReportCallbackEXT)                                                             \
    F(vkDebugReportMessageEXT)                                                                     \
    F(vkDebugMarkerSetObjectTagEXT)                                                                \
    F(vkDebugMarkerSetObjectNameEXT)                                                               \
    F(vkCmdDebugMarkerBeginEXT)                                                                    \
    F(vkCmdDebugMarkerEndEXT)                                                                      \
    F(vkCmdDebugMarkerInsertEXT)                                                                   \
    F(vkCmdBindTransformFeedbackBuffersEXT)                                                        \
    F(vkCmdBeginTransformFeedbackEXT)                                                              \
    F(vkCmdEndTransformFeedbackEXT)                                                                \
    F(vkCmdBeginQueryIndexedEXT)                                                                   \
    F(vkCmdEndQueryIndexedEXT)                                                                     \
    F(vkCmdDrawIndirectByteCountEXT)                                                               \
    F(vkCreateCuModuleNVX)                                                                         \
    F(vkCreateCuFunctionNVX)                                                                       \
    F(vkDestroyCuModuleNVX)                                                                        \
    F(vkDestroyCuFunctionNVX)                                                                      \
    F(vkCmdCuLaunchKernelNVX)                                                                      \
    F(vkGetImageViewHandleNVX)                                                                     \
    F(vkGetImageViewHandle64NVX)                                                                   \
    F(vkGetImageViewAddressNVX)                                                                    \
    F(vkCmdDrawIndirectCountAMD)                                                                   \
    F(vkCmdDrawIndexedIndirectCountAMD)                                                            \
    F(vkGetShaderInfoAMD)                                                                          \
    F(vkGetPhysicalDeviceExternalImageFormatPropertiesNV)                                          \
    F(vkCmdBeginConditionalRenderingEXT)                                                           \
    F(vkCmdEndConditionalRenderingEXT)                                                             \
    F(vkCmdSetViewportWScalingNV)                                                                  \
    F(vkReleaseDisplayEXT)                                                                         \
    F(vkGetPhysicalDeviceSurfaceCapabilities2EXT)                                                  \
    F(vkDisplayPowerControlEXT)                                                                    \
    F(vkRegisterDeviceEventEXT)                                                                    \
    F(vkRegisterDisplayEventEXT)                                                                   \
    F(vkGetSwapchainCounterEXT)                                                                    \
    F(vkGetRefreshCycleDurationGOOGLE)                                                             \
    F(vkGetPastPresentationTimingGOOGLE)                                                           \
    F(vkCmdSetDiscardRectangleEXT)                                                                 \
    F(vkCmdSetDiscardRectangleEnableEXT)                                                           \
    F(vkCmdSetDiscardRectangleModeEXT)                                                             \
    F(vkSetHdrMetadataEXT)                                                                         \
    F(vkSetDebugUtilsObjectNameEXT)                                                                \
    F(vkSetDebugUtilsObjectTagEXT)                                                                 \
    F(vkQueueBeginDebugUtilsLabelEXT)                                                              \
    F(vkQueueEndDebugUtilsLabelEXT)                                                                \
    F(vkQueueInsertDebugUtilsLabelEXT)                                                             \
    F(vkCmdBeginDebugUtilsLabelEXT)                                                                \
    F(vkCmdEndDebugUtilsLabelEXT)                                                                  \
    F(vkCmdInsertDebugUtilsLabelEXT)                                                               \
    F(vkCreateDebugUtilsMessengerEXT)                                                              \
    F(vkDestroyDebugUtilsMessengerEXT)                                                             \
    F(vkSubmitDebugUtilsMessageEXT)                                                                \
    F(vkCmdSetSampleLocationsEXT)                                                                  \
    F(vkGetPhysicalDeviceMultisamplePropertiesEXT)                                                 \
    F(vkGetImageDrmFormatModifierPropertiesEXT)                                                    \
    F(vkCreateValidationCacheEXT)                                                                  \
    F(vkDestroyValidationCacheEXT)                                                                 \
    F(vkMergeValidationCachesEXT)                                                                  \
    F(vkGetValidationCacheDataEXT)                                                                 \
    F(vkCmdBindShadingRateImageNV)                                                                 \
    F(vkCmdSetViewportShadingRatePaletteNV)                                                        \
    F(vkCmdSetCoarseSampleOrderNV)                                                                 \
    F(vkCreateAccelerationStructureNV)                                                             \
    F(vkDestroyAccelerationStructureNV)                                                            \
    F(vkGetAccelerationStructureMemoryRequirementsNV)                                              \
    F(vkBindAccelerationStructureMemoryNV)                                                         \
    F(vkCmdBuildAccelerationStructureNV)                                                           \
    F(vkCmdCopyAccelerationStructureNV)                                                            \
    F(vkCmdTraceRaysNV)                                                                            \
    F(vkCreateRayTracingPipelinesNV)                                                               \
    F(vkGetRayTracingShaderGroupHandlesKHR)                                                        \
    F(vkGetRayTracingShaderGroupHandlesNV)                                                         \
    F(vkGetAccelerationStructureHandleNV)                                                          \
    F(vkCmdWriteAccelerationStructuresPropertiesNV)                                                \
    F(vkCompileDeferredNV)                                                                         \
    F(vkGetMemoryHostPointerPropertiesEXT)                                                         \
    F(vkCmdWriteBufferMarkerAMD)                                                                   \
    F(vkCmdWriteBufferMarker2AMD)                                                                  \
    F(vkGetPhysicalDeviceCalibrateableTimeDomainsEXT)                                              \
    F(vkGetCalibratedTimestampsEXT)                                                                \
    F(vkCmdDrawMeshTasksNV)                                                                        \
    F(vkCmdDrawMeshTasksIndirectNV)                                                                \
    F(vkCmdDrawMeshTasksIndirectCountNV)                                                           \
    F(vkCmdSetExclusiveScissorEnableNV)                                                            \
    F(vkCmdSetExclusiveScissorNV)                                                                  \
    F(vkCmdSetCheckpointNV)                                                                        \
    F(vkGetQueueCheckpointDataNV)                                                                  \
    F(vkGetQueueCheckpointData2NV)                                                                 \
    F(vkInitializePerformanceApiINTEL)                                                             \
    F(vkUninitializePerformanceApiINTEL)                                                           \
    F(vkCmdSetPerformanceMarkerINTEL)                                                              \
    F(vkCmdSetPerformanceStreamMarkerINTEL)                                                        \
    F(vkCmdSetPerformanceOverrideINTEL)                                                            \
    F(vkAcquirePerformanceConfigurationINTEL)                                                      \
    F(vkReleasePerformanceConfigurationINTEL)                                                      \
    F(vkQueueSetPerformanceConfigurationINTEL)                                                     \
    F(vkGetPerformanceParameterINTEL)                                                              \
    F(vkSetLocalDimmingAMD)                                                                        \
    F(vkGetBufferDeviceAddressEXT)                                                                 \
    F(vkGetPhysicalDeviceToolPropertiesEXT)                                                        \
    F(vkGetPhysicalDeviceCooperativeMatrixPropertiesNV)                                            \
    F(vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV)                           \
    F(vkCreateHeadlessSurfaceEXT)                                                                  \
    F(vkCmdSetLineStippleEXT)                                                                      \
    F(vkResetQueryPoolEXT)                                                                         \
    F(vkCmdSetCullModeEXT)                                                                         \
    F(vkCmdSetFrontFaceEXT)                                                                        \
    F(vkCmdSetPrimitiveTopologyEXT)                                                                \
    F(vkCmdSetViewportWithCountEXT)                                                                \
    F(vkCmdSetScissorWithCountEXT)                                                                 \
    F(vkCmdBindVertexBuffers2EXT)                                                                  \
    F(vkCmdSetDepthTestEnableEXT)                                                                  \
    F(vkCmdSetDepthWriteEnableEXT)                                                                 \
    F(vkCmdSetDepthCompareOpEXT)                                                                   \
    F(vkCmdSetDepthBoundsTestEnableEXT)                                                            \
    F(vkCmdSetStencilTestEnableEXT)                                                                \
    F(vkCmdSetStencilOpEXT)                                                                        \
    F(vkCopyMemoryToImageEXT)                                                                      \
    F(vkCopyImageToMemoryEXT)                                                                      \
    F(vkCopyImageToImageEXT)                                                                       \
    F(vkTransitionImageLayoutEXT)                                                                  \
    F(vkGetImageSubresourceLayout2EXT)                                                             \
    F(vkReleaseSwapchainImagesEXT)                                                                 \
    F(vkGetGeneratedCommandsMemoryRequirementsNV)                                                  \
    F(vkCmdPreprocessGeneratedCommandsNV)                                                          \
    F(vkCmdExecuteGeneratedCommandsNV)                                                             \
    F(vkCmdBindPipelineShaderGroupNV)                                                              \
    F(vkCreateIndirectCommandsLayoutNV)                                                            \
    F(vkDestroyIndirectCommandsLayoutNV)                                                           \
    F(vkCmdSetDepthBias2EXT)                                                                       \
    F(vkAcquireDrmDisplayEXT)                                                                      \
    F(vkGetDrmDisplayEXT)                                                                          \
    F(vkCreatePrivateDataSlotEXT)                                                                  \
    F(vkDestroyPrivateDataSlotEXT)                                                                 \
    F(vkSetPrivateDataEXT)                                                                         \
    F(vkGetPrivateDataEXT)                                                                         \
    F(vkCmdDispatchTileQCOM)                                                                       \
    F(vkCmdBeginPerTileExecutionQCOM)                                                              \
    F(vkCmdEndPerTileExecutionQCOM)                                                                \
    F(vkGetDescriptorSetLayoutSizeEXT)                                                             \
    F(vkGetDescriptorSetLayoutBindingOffsetEXT)                                                    \
    F(vkGetDescriptorEXT)                                                                          \
    F(vkCmdBindDescriptorBuffersEXT)                                                               \
    F(vkCmdSetDescriptorBufferOffsetsEXT)                                                          \
    F(vkCmdBindDescriptorBufferEmbeddedSamplersEXT)                                                \
    F(vkGetBufferOpaqueCaptureDescriptorDataEXT)                                                   \
    F(vkGetImageOpaqueCaptureDescriptorDataEXT)                                                    \
    F(vkGetImageViewOpaqueCaptureDescriptorDataEXT)                                                \
    F(vkGetSamplerOpaqueCaptureDescriptorDataEXT)                                                  \
    F(vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT)                                    \
    F(vkCmdSetFragmentShadingRateEnumNV)                                                           \
    F(vkGetDeviceFaultInfoEXT)                                                                     \
    F(vkCmdSetVertexInputEXT)                                                                      \
    F(vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI)                                             \
    F(vkCmdSubpassShadingHUAWEI)                                                                   \
    F(vkCmdBindInvocationMaskHUAWEI)                                                               \
    F(vkGetMemoryRemoteAddressNV)                                                                  \
    F(vkGetPipelinePropertiesEXT)                                                                  \
    F(vkCmdSetPatchControlPointsEXT)                                                               \
    F(vkCmdSetRasterizerDiscardEnableEXT)                                                          \
    F(vkCmdSetDepthBiasEnableEXT)                                                                  \
    F(vkCmdSetLogicOpEXT)                                                                          \
    F(vkCmdSetPrimitiveRestartEnableEXT)                                                           \
    F(vkCmdSetColorWriteEnableEXT)                                                                 \
    F(vkCmdDrawMultiEXT)                                                                           \
    F(vkCmdDrawMultiIndexedEXT)                                                                    \
    F(vkCreateMicromapEXT)                                                                         \
    F(vkDestroyMicromapEXT)                                                                        \
    F(vkCmdBuildMicromapsEXT)                                                                      \
    F(vkBuildMicromapsEXT)                                                                         \
    F(vkCopyMicromapEXT)                                                                           \
    F(vkCopyMicromapToMemoryEXT)                                                                   \
    F(vkCopyMemoryToMicromapEXT)                                                                   \
    F(vkWriteMicromapsPropertiesEXT)                                                               \
    F(vkCmdCopyMicromapEXT)                                                                        \
    F(vkCmdCopyMicromapToMemoryEXT)                                                                \
    F(vkCmdCopyMemoryToMicromapEXT)                                                                \
    F(vkCmdWriteMicromapsPropertiesEXT)                                                            \
    F(vkGetDeviceMicromapCompatibilityEXT)                                                         \
    F(vkGetMicromapBuildSizesEXT)                                                                  \
    F(vkCmdDrawClusterHUAWEI)                                                                      \
    F(vkCmdDrawClusterIndirectHUAWEI)                                                              \
    F(vkSetDeviceMemoryPriorityEXT)                                                                \
    F(vkGetDescriptorSetLayoutHostMappingInfoVALVE)                                                \
    F(vkGetDescriptorSetHostMappingVALVE)                                                          \
    F(vkCmdCopyMemoryIndirectNV)                                                                   \
    F(vkCmdCopyMemoryToImageIndirectNV)                                                            \
    F(vkCmdDecompressMemoryNV)                                                                     \
    F(vkCmdDecompressMemoryIndirectCountNV)                                                        \
    F(vkGetPipelineIndirectMemoryRequirementsNV)                                                   \
    F(vkCmdUpdatePipelineIndirectBufferNV)                                                         \
    F(vkGetPipelineIndirectDeviceAddressNV)                                                        \
    F(vkCmdSetDepthClampEnableEXT)                                                                 \
    F(vkCmdSetPolygonModeEXT)                                                                      \
    F(vkCmdSetRasterizationSamplesEXT)                                                             \
    F(vkCmdSetSampleMaskEXT)                                                                       \
    F(vkCmdSetAlphaToCoverageEnableEXT)                                                            \
    F(vkCmdSetAlphaToOneEnableEXT)                                                                 \
    F(vkCmdSetLogicOpEnableEXT)                                                                    \
    F(vkCmdSetColorBlendEnableEXT)                                                                 \
    F(vkCmdSetColorBlendEquationEXT)                                                               \
    F(vkCmdSetColorWriteMaskEXT)                                                                   \
    F(vkCmdSetTessellationDomainOriginEXT)                                                         \
    F(vkCmdSetRasterizationStreamEXT)                                                              \
    F(vkCmdSetConservativeRasterizationModeEXT)                                                    \
    F(vkCmdSetExtraPrimitiveOverestimationSizeEXT)                                                 \
    F(vkCmdSetDepthClipEnableEXT)                                                                  \
    F(vkCmdSetSampleLocationsEnableEXT)                                                            \
    F(vkCmdSetColorBlendAdvancedEXT)                                                               \
    F(vkCmdSetProvokingVertexModeEXT)                                                              \
    F(vkCmdSetLineRasterizationModeEXT)                                                            \
    F(vkCmdSetLineStippleEnableEXT)                                                                \
    F(vkCmdSetDepthClipNegativeOneToOneEXT)                                                        \
    F(vkCmdSetViewportWScalingEnableNV)                                                            \
    F(vkCmdSetViewportSwizzleNV)                                                                   \
    F(vkCmdSetCoverageToColorEnableNV)                                                             \
    F(vkCmdSetCoverageToColorLocationNV)                                                           \
    F(vkCmdSetCoverageModulationModeNV)                                                            \
    F(vkCmdSetCoverageModulationTableEnableNV)                                                     \
    F(vkCmdSetCoverageModulationTableNV)                                                           \
    F(vkCmdSetShadingRateImageEnableNV)                                                            \
    F(vkCmdSetRepresentativeFragmentTestEnableNV)                                                  \
    F(vkCmdSetCoverageReductionModeNV)                                                             \
    F(vkGetShaderModuleIdentifierEXT)                                                              \
    F(vkGetShaderModuleCreateInfoIdentifierEXT)                                                    \
    F(vkGetPhysicalDeviceOpticalFlowImageFormatsNV)                                                \
    F(vkCreateOpticalFlowSessionNV)                                                                \
    F(vkDestroyOpticalFlowSessionNV)                                                               \
    F(vkBindOpticalFlowSessionImageNV)                                                             \
    F(vkCmdOpticalFlowExecuteNV)                                                                   \
    F(vkAntiLagUpdateAMD)                                                                          \
    F(vkCreateShadersEXT)                                                                          \
    F(vkDestroyShaderEXT)                                                                          \
    F(vkGetShaderBinaryDataEXT)                                                                    \
    F(vkCmdBindShadersEXT)                                                                         \
    F(vkCmdSetDepthClampRangeEXT)                                                                  \
    F(vkGetFramebufferTilePropertiesQCOM)                                                          \
    F(vkGetDynamicRenderingTilePropertiesQCOM)                                                     \
    F(vkGetPhysicalDeviceCooperativeVectorPropertiesNV)                                            \
    F(vkConvertCooperativeVectorMatrixNV)                                                          \
    F(vkCmdConvertCooperativeVectorMatrixNV)                                                       \
    F(vkSetLatencySleepModeNV)                                                                     \
    F(vkLatencySleepNV)                                                                            \
    F(vkSetLatencyMarkerNV)                                                                        \
    F(vkGetLatencyTimingsNV)                                                                       \
    F(vkQueueNotifyOutOfBandNV)                                                                    \
    F(vkCmdSetAttachmentFeedbackLoopEnableEXT)                                                     \
    F(vkCmdBindTileMemoryQCOM)                                                                     \
    F(vkCreateExternalComputeQueueNV)                                                              \
    F(vkDestroyExternalComputeQueueNV)                                                             \
    F(vkGetExternalComputeQueueDataNV)                                                             \
    F(vkGetClusterAccelerationStructureBuildSizesNV)                                               \
    F(vkCmdBuildClusterAccelerationStructureIndirectNV)                                            \
    F(vkGetPartitionedAccelerationStructuresBuildSizesNV)                                          \
    F(vkCmdBuildPartitionedAccelerationStructuresNV)                                               \
    F(vkGetGeneratedCommandsMemoryRequirementsEXT)                                                 \
    F(vkCmdPreprocessGeneratedCommandsEXT)                                                         \
    F(vkCmdExecuteGeneratedCommandsEXT)                                                            \
    F(vkCreateIndirectCommandsLayoutEXT)                                                           \
    F(vkDestroyIndirectCommandsLayoutEXT)                                                          \
    F(vkCreateIndirectExecutionSetEXT)                                                             \
    F(vkDestroyIndirectExecutionSetEXT)                                                            \
    F(vkUpdateIndirectExecutionSetPipelineEXT)                                                     \
    F(vkUpdateIndirectExecutionSetShaderEXT)                                                       \
    F(vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV)                          \
    F(vkCmdEndRendering2EXT)                                                                       \
    F(vkCreateAccelerationStructureKHR)                                                            \
    F(vkDestroyAccelerationStructureKHR)                                                           \
    F(vkCmdBuildAccelerationStructuresKHR)                                                         \
    F(vkCmdBuildAccelerationStructuresIndirectKHR)                                                 \
    F(vkBuildAccelerationStructuresKHR)                                                            \
    F(vkCopyAccelerationStructureKHR)                                                              \
    F(vkCopyAccelerationStructureToMemoryKHR)                                                      \
    F(vkCopyMemoryToAccelerationStructureKHR)                                                      \
    F(vkWriteAccelerationStructuresPropertiesKHR)                                                  \
    F(vkCmdCopyAccelerationStructureKHR)                                                           \
    F(vkCmdCopyAccelerationStructureToMemoryKHR)                                                   \
    F(vkCmdCopyMemoryToAccelerationStructureKHR)                                                   \
    F(vkGetAccelerationStructureDeviceAddressKHR)                                                  \
    F(vkCmdWriteAccelerationStructuresPropertiesKHR)                                               \
    F(vkGetDeviceAccelerationStructureCompatibilityKHR)                                            \
    F(vkGetAccelerationStructureBuildSizesKHR)                                                     \
    F(vkCmdTraceRaysKHR)                                                                           \
    F(vkCreateRayTracingPipelinesKHR)                                                              \
    F(vkGetRayTracingCaptureReplayShaderGroupHandlesKHR)                                           \
    F(vkCmdTraceRaysIndirectKHR)                                                                   \
    F(vkGetRayTracingShaderGroupStackSizeKHR)                                                      \
    F(vkCmdSetRayTracingPipelineStackSizeKHR)                                                      \
    F(vkCmdDrawMeshTasksEXT)                                                                       \
    F(vkCmdDrawMeshTasksIndirectEXT)                                                               \
    F(vkCmdDrawMeshTasksIndirectCountEXT)                                                          \
    F(vkCreateWaylandSurfaceKHR)                                                                   \
    F(vkGetPhysicalDeviceWaylandPresentationSupportKHR)                                            \
    F(vkCreateXcbSurfaceKHR)                                                                       \
    F(vkGetPhysicalDeviceXcbPresentationSupportKHR)                                                \
    F(vkCreateXlibSurfaceKHR)                                                                      \
    F(vkGetPhysicalDeviceXlibPresentationSupportKHR)


// GCBs
static VkResult __HINT_GCB_PFN_vkCreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkInstance *pInstance);
static void __HINT_GCB_PFN_vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_GCB_PFN_vkEnumeratePhysicalDevices(VkInstance instance, uint32_t *pPhysicalDeviceCount, VkPhysicalDevice *pPhysicalDevices);
static void __HINT_GCB_PFN_vkGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures *pFeatures);
static void __HINT_GCB_PFN_vkGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties *pFormatProperties);
static VkResult __HINT_GCB_PFN_vkGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkImageFormatProperties *pImageFormatProperties);
static void __HINT_GCB_PFN_vkGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties *pProperties);
static void __HINT_GCB_PFN_vkGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t *pQueueFamilyPropertyCount, VkQueueFamilyProperties *pQueueFamilyProperties);
static void __HINT_GCB_PFN_vkGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties *pMemoryProperties);
static PFN_vkVoidFunction __HINT_GCB_PFN_vkGetInstanceProcAddr(VkInstance instance, const char *pName);
static PFN_vkVoidFunction __HINT_GCB_PFN_vkGetDeviceProcAddr(VkDevice device, const char *pName);
static VkResult __HINT_GCB_PFN_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDevice *pDevice);
static void __HINT_GCB_PFN_vkDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_GCB_PFN_vkEnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pPropertyCount, VkExtensionProperties *pProperties);
static VkResult __HINT_GCB_PFN_vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char *pLayerName, uint32_t *pPropertyCount, VkExtensionProperties *pProperties);
static VkResult __HINT_GCB_PFN_vkEnumerateInstanceLayerProperties(uint32_t *pPropertyCount, VkLayerProperties *pProperties);
static VkResult __HINT_GCB_PFN_vkEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount, VkLayerProperties *pProperties);
static void __HINT_GCB_PFN_vkGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue *pQueue);
static VkResult __HINT_GCB_PFN_vkQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence);
static VkResult __HINT_GCB_PFN_vkQueueWaitIdle(VkQueue queue);
static VkResult __HINT_GCB_PFN_vkDeviceWaitIdle(VkDevice device);
static VkResult __HINT_GCB_PFN_vkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo, const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory);
static void __HINT_GCB_PFN_vkFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_GCB_PFN_vkMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void **ppData);
static void __HINT_GCB_PFN_vkUnmapMemory(VkDevice device, VkDeviceMemory memory);
static VkResult __HINT_GCB_PFN_vkFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange *pMemoryRanges);
static VkResult __HINT_GCB_PFN_vkInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange *pMemoryRanges);
static void __HINT_GCB_PFN_vkGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize *pCommittedMemoryInBytes);
static VkResult __HINT_GCB_PFN_vkBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset);
static VkResult __HINT_GCB_PFN_vkBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset);
static void __HINT_GCB_PFN_vkGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements *pMemoryRequirements);
static void __HINT_GCB_PFN_vkGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements *pMemoryRequirements);
static void __HINT_GCB_PFN_vkGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t *pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements *pSparseMemoryRequirements);
static void __HINT_GCB_PFN_vkGetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t *pPropertyCount, VkSparseImageFormatProperties *pProperties);
static VkResult __HINT_GCB_PFN_vkQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo *pBindInfo, VkFence fence);
static VkResult __HINT_GCB_PFN_vkCreateFence(VkDevice device, const VkFenceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkFence *pFence);
static void __HINT_GCB_PFN_vkDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_GCB_PFN_vkResetFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences);
static VkResult __HINT_GCB_PFN_vkGetFenceStatus(VkDevice device, VkFence fence);
static VkResult __HINT_GCB_PFN_vkWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences, VkBool32 waitAll, uint64_t timeout);
static VkResult __HINT_GCB_PFN_vkCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSemaphore *pSemaphore);
static void __HINT_GCB_PFN_vkDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_GCB_PFN_vkCreateEvent(VkDevice device, const VkEventCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkEvent *pEvent);
static void __HINT_GCB_PFN_vkDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_GCB_PFN_vkGetEventStatus(VkDevice device, VkEvent event);
static VkResult __HINT_GCB_PFN_vkSetEvent(VkDevice device, VkEvent event);
static VkResult __HINT_GCB_PFN_vkResetEvent(VkDevice device, VkEvent event);
static VkResult __HINT_GCB_PFN_vkCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkQueryPool *pQueryPool);
static void __HINT_GCB_PFN_vkDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_GCB_PFN_vkGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void *pData, VkDeviceSize stride, VkQueryResultFlags flags);
static VkResult __HINT_GCB_PFN_vkCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer);
static void __HINT_GCB_PFN_vkDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_GCB_PFN_vkCreateBufferView(VkDevice device, const VkBufferViewCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkBufferView *pView);
static void __HINT_GCB_PFN_vkDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_GCB_PFN_vkCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkImage *pImage);
static void __HINT_GCB_PFN_vkDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator);
static void __HINT_GCB_PFN_vkGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource *pSubresource, VkSubresourceLayout *pLayout);
static VkResult __HINT_GCB_PFN_vkCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkImageView *pView);
static void __HINT_GCB_PFN_vkDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_GCB_PFN_vkCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkShaderModule *pShaderModule);
static void __HINT_GCB_PFN_vkDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_GCB_PFN_vkCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkPipelineCache *pPipelineCache);
static void __HINT_GCB_PFN_vkDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_GCB_PFN_vkGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t *pDataSize, void *pData);
static VkResult __HINT_GCB_PFN_vkMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount, const VkPipelineCache *pSrcCaches);
static VkResult __HINT_GCB_PFN_vkCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines);
static VkResult __HINT_GCB_PFN_vkCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines);
static void __HINT_GCB_PFN_vkDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_GCB_PFN_vkCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout);
static void __HINT_GCB_PFN_vkDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_GCB_PFN_vkCreateSampler(VkDevice device, const VkSamplerCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSampler *pSampler);
static void __HINT_GCB_PFN_vkDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_GCB_PFN_vkCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDescriptorSetLayout *pSetLayout);
static void __HINT_GCB_PFN_vkDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_GCB_PFN_vkCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDescriptorPool *pDescriptorPool);
static void __HINT_GCB_PFN_vkDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_GCB_PFN_vkResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags);
static VkResult __HINT_GCB_PFN_vkAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo *pAllocateInfo, VkDescriptorSet *pDescriptorSets);
static VkResult __HINT_GCB_PFN_vkFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet *pDescriptorSets);
static void __HINT_GCB_PFN_vkUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount, const VkWriteDescriptorSet *pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet *pDescriptorCopies);
static VkResult __HINT_GCB_PFN_vkCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkFramebuffer *pFramebuffer);
static void __HINT_GCB_PFN_vkDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_GCB_PFN_vkCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass);
static void __HINT_GCB_PFN_vkDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks *pAllocator);
static void __HINT_GCB_PFN_vkGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D *pGranularity);
static VkResult __HINT_GCB_PFN_vkCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkCommandPool *pCommandPool);
static void __HINT_GCB_PFN_vkDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_GCB_PFN_vkResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags);
static VkResult __HINT_GCB_PFN_vkAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo *pAllocateInfo, VkCommandBuffer *pCommandBuffers);
static void __HINT_GCB_PFN_vkFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer *pCommandBuffers);
static VkResult __HINT_GCB_PFN_vkBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *pBeginInfo);
static VkResult __HINT_GCB_PFN_vkEndCommandBuffer(VkCommandBuffer commandBuffer);
static VkResult __HINT_GCB_PFN_vkResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags);
static void __HINT_GCB_PFN_vkCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);
static void __HINT_GCB_PFN_vkCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport *pViewports);
static void __HINT_GCB_PFN_vkCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D *pScissors);
static void __HINT_GCB_PFN_vkCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth);
static void __HINT_GCB_PFN_vkCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor);
static void __HINT_GCB_PFN_vkCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]);
static void __HINT_GCB_PFN_vkCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds);
static void __HINT_GCB_PFN_vkCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask);
static void __HINT_GCB_PFN_vkCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask);
static void __HINT_GCB_PFN_vkCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference);
static void __HINT_GCB_PFN_vkCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet *pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t *pDynamicOffsets);
static void __HINT_GCB_PFN_vkCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType);
static void __HINT_GCB_PFN_vkCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer *pBuffers, const VkDeviceSize *pOffsets);
static void __HINT_GCB_PFN_vkCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
static void __HINT_GCB_PFN_vkCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
static void __HINT_GCB_PFN_vkCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
static void __HINT_GCB_PFN_vkCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
static void __HINT_GCB_PFN_vkCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
static void __HINT_GCB_PFN_vkCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset);
static void __HINT_GCB_PFN_vkCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy *pRegions);
static void __HINT_GCB_PFN_vkCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy *pRegions);
static void __HINT_GCB_PFN_vkCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit *pRegions, VkFilter filter);
static void __HINT_GCB_PFN_vkCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy *pRegions);
static void __HINT_GCB_PFN_vkCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions);
static void __HINT_GCB_PFN_vkCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void *pData);
static void __HINT_GCB_PFN_vkCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data);
static void __HINT_GCB_PFN_vkCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue *pColor, uint32_t rangeCount, const VkImageSubresourceRange *pRanges);
static void __HINT_GCB_PFN_vkCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange *pRanges);
static void __HINT_GCB_PFN_vkCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment *pAttachments, uint32_t rectCount, const VkClearRect *pRects);
static void __HINT_GCB_PFN_vkCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve *pRegions);
static void __HINT_GCB_PFN_vkCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask);
static void __HINT_GCB_PFN_vkCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask);
static void __HINT_GCB_PFN_vkCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers);
static void __HINT_GCB_PFN_vkCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers);
static void __HINT_GCB_PFN_vkCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags);
static void __HINT_GCB_PFN_vkCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query);
static void __HINT_GCB_PFN_vkCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);
static void __HINT_GCB_PFN_vkCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query);
static void __HINT_GCB_PFN_vkCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags);
static void __HINT_GCB_PFN_vkCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void *pValues);
static void __HINT_GCB_PFN_vkCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin, VkSubpassContents contents);
static void __HINT_GCB_PFN_vkCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents);
static void __HINT_GCB_PFN_vkCmdEndRenderPass(VkCommandBuffer commandBuffer);
static void __HINT_GCB_PFN_vkCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount, const VkCommandBuffer *pCommandBuffers);
static VkResult __HINT_GCB_PFN_vkEnumerateInstanceVersion(uint32_t *pApiVersion);
static VkResult __HINT_GCB_PFN_vkBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo *pBindInfos);
static VkResult __HINT_GCB_PFN_vkBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo *pBindInfos);
static void __HINT_GCB_PFN_vkGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags *pPeerMemoryFeatures);
static void __HINT_GCB_PFN_vkCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask);
static void __HINT_GCB_PFN_vkCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
static VkResult __HINT_GCB_PFN_vkEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t *pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroupProperties);
static void __HINT_GCB_PFN_vkGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2 *pInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __HINT_GCB_PFN_vkGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2 *pInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __HINT_GCB_PFN_vkGetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2 *pInfo, uint32_t *pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements);
static void __HINT_GCB_PFN_vkGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2 *pFeatures);
static void __HINT_GCB_PFN_vkGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2 *pProperties);
static void __HINT_GCB_PFN_vkGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2 *pFormatProperties);
static VkResult __HINT_GCB_PFN_vkGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo, VkImageFormatProperties2 *pImageFormatProperties);
static void __HINT_GCB_PFN_vkGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t *pQueueFamilyPropertyCount, VkQueueFamilyProperties2 *pQueueFamilyProperties);
static void __HINT_GCB_PFN_vkGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2 *pMemoryProperties);
static void __HINT_GCB_PFN_vkGetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2 *pFormatInfo, uint32_t *pPropertyCount, VkSparseImageFormatProperties2 *pProperties);
static void __HINT_GCB_PFN_vkTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags);
static void __HINT_GCB_PFN_vkGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2 *pQueueInfo, VkQueue *pQueue);
static VkResult __HINT_GCB_PFN_vkCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSamplerYcbcrConversion *pYcbcrConversion);
static void __HINT_GCB_PFN_vkDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_GCB_PFN_vkCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate);
static void __HINT_GCB_PFN_vkDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks *pAllocator);
static void __HINT_GCB_PFN_vkUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void *pData);
static void __HINT_GCB_PFN_vkGetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo *pExternalBufferInfo, VkExternalBufferProperties *pExternalBufferProperties);
static void __HINT_GCB_PFN_vkGetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo *pExternalFenceInfo, VkExternalFenceProperties *pExternalFenceProperties);
static void __HINT_GCB_PFN_vkGetPhysicalDeviceExternalSemaphoreProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo *pExternalSemaphoreInfo, VkExternalSemaphoreProperties *pExternalSemaphoreProperties);
static void __HINT_GCB_PFN_vkGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo, VkDescriptorSetLayoutSupport *pSupport);
static void __HINT_GCB_PFN_vkCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);
static void __HINT_GCB_PFN_vkCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);
static VkResult __HINT_GCB_PFN_vkCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass);
static void __HINT_GCB_PFN_vkCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin, const VkSubpassBeginInfo *pSubpassBeginInfo);
static void __HINT_GCB_PFN_vkCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo, const VkSubpassEndInfo *pSubpassEndInfo);
static void __HINT_GCB_PFN_vkCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo);
static void __HINT_GCB_PFN_vkResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);
static VkResult __HINT_GCB_PFN_vkGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t *pValue);
static VkResult __HINT_GCB_PFN_vkWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout);
static VkResult __HINT_GCB_PFN_vkSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo);
static VkDeviceAddress __HINT_GCB_PFN_vkGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo *pInfo);
static uint64_t __HINT_GCB_PFN_vkGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo *pInfo);
static uint64_t __HINT_GCB_PFN_vkGetDeviceMemoryOpaqueCaptureAddress(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo *pInfo);
static VkResult __HINT_GCB_PFN_vkGetPhysicalDeviceToolProperties(VkPhysicalDevice physicalDevice, uint32_t *pToolCount, VkPhysicalDeviceToolProperties *pToolProperties);
static VkResult __HINT_GCB_PFN_vkCreatePrivateDataSlot(VkDevice device, const VkPrivateDataSlotCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkPrivateDataSlot *pPrivateDataSlot);
static void __HINT_GCB_PFN_vkDestroyPrivateDataSlot(VkDevice device, VkPrivateDataSlot privateDataSlot, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_GCB_PFN_vkSetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlot privateDataSlot, uint64_t data);
static void __HINT_GCB_PFN_vkGetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlot privateDataSlot, uint64_t *pData);
static void __HINT_GCB_PFN_vkCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo *pDependencyInfo);
static void __HINT_GCB_PFN_vkCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask);
static void __HINT_GCB_PFN_vkCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents, const VkDependencyInfo *pDependencyInfos);
static void __HINT_GCB_PFN_vkCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo);
static void __HINT_GCB_PFN_vkCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool, uint32_t query);
static VkResult __HINT_GCB_PFN_vkQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits, VkFence fence);
static void __HINT_GCB_PFN_vkCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo);
static void __HINT_GCB_PFN_vkCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo);
static void __HINT_GCB_PFN_vkCmdCopyBufferToImage2(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo);
static void __HINT_GCB_PFN_vkCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo);
static void __HINT_GCB_PFN_vkCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2 *pBlitImageInfo);
static void __HINT_GCB_PFN_vkCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2 *pResolveImageInfo);
static void __HINT_GCB_PFN_vkCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo);
static void __HINT_GCB_PFN_vkCmdEndRendering(VkCommandBuffer commandBuffer);
static void __HINT_GCB_PFN_vkCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode);
static void __HINT_GCB_PFN_vkCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace);
static void __HINT_GCB_PFN_vkCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology);
static void __HINT_GCB_PFN_vkCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport *pViewports);
static void __HINT_GCB_PFN_vkCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D *pScissors);
static void __HINT_GCB_PFN_vkCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer *pBuffers, const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes, const VkDeviceSize *pStrides);
static void __HINT_GCB_PFN_vkCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable);
static void __HINT_GCB_PFN_vkCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable);
static void __HINT_GCB_PFN_vkCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp);
static void __HINT_GCB_PFN_vkCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable);
static void __HINT_GCB_PFN_vkCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable);
static void __HINT_GCB_PFN_vkCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp);
static void __HINT_GCB_PFN_vkCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable);
static void __HINT_GCB_PFN_vkCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable);
static void __HINT_GCB_PFN_vkCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable);
static void __HINT_GCB_PFN_vkGetDeviceBufferMemoryRequirements(VkDevice device, const VkDeviceBufferMemoryRequirements *pInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __HINT_GCB_PFN_vkGetDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements *pInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __HINT_GCB_PFN_vkGetDeviceImageSparseMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements *pInfo, uint32_t *pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements);
static void __HINT_GCB_PFN_vkCmdSetLineStipple(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern);
static VkResult __HINT_GCB_PFN_vkMapMemory2(VkDevice device, const VkMemoryMapInfo *pMemoryMapInfo, void **ppData);
static VkResult __HINT_GCB_PFN_vkUnmapMemory2(VkDevice device, const VkMemoryUnmapInfo *pMemoryUnmapInfo);
static void __HINT_GCB_PFN_vkCmdBindIndexBuffer2(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size, VkIndexType indexType);
static void __HINT_GCB_PFN_vkGetRenderingAreaGranularity(VkDevice device, const VkRenderingAreaInfo *pRenderingAreaInfo, VkExtent2D *pGranularity);
static void __HINT_GCB_PFN_vkGetDeviceImageSubresourceLayout(VkDevice device, const VkDeviceImageSubresourceInfo *pInfo, VkSubresourceLayout2 *pLayout);
static void __HINT_GCB_PFN_vkGetImageSubresourceLayout2(VkDevice device, VkImage image, const VkImageSubresource2 *pSubresource, VkSubresourceLayout2 *pLayout);
static void __HINT_GCB_PFN_vkCmdPushDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount, const VkWriteDescriptorSet *pDescriptorWrites);
static void __HINT_GCB_PFN_vkCmdPushDescriptorSetWithTemplate(VkCommandBuffer commandBuffer, VkDescriptorUpdateTemplate descriptorUpdateTemplate, VkPipelineLayout layout, uint32_t set, const void *pData);
static void __HINT_GCB_PFN_vkCmdSetRenderingAttachmentLocations(VkCommandBuffer commandBuffer, const VkRenderingAttachmentLocationInfo *pLocationInfo);
static void __HINT_GCB_PFN_vkCmdSetRenderingInputAttachmentIndices(VkCommandBuffer commandBuffer, const VkRenderingInputAttachmentIndexInfo *pInputAttachmentIndexInfo);
static void __HINT_GCB_PFN_vkCmdBindDescriptorSets2(VkCommandBuffer commandBuffer, const VkBindDescriptorSetsInfo *pBindDescriptorSetsInfo);
static void __HINT_GCB_PFN_vkCmdPushConstants2(VkCommandBuffer commandBuffer, const VkPushConstantsInfo *pPushConstantsInfo);
static void __HINT_GCB_PFN_vkCmdPushDescriptorSet2(VkCommandBuffer commandBuffer, const VkPushDescriptorSetInfo *pPushDescriptorSetInfo);
static void __HINT_GCB_PFN_vkCmdPushDescriptorSetWithTemplate2(VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfo *pPushDescriptorSetWithTemplateInfo);
static VkResult __HINT_GCB_PFN_vkCopyMemoryToImage(VkDevice device, const VkCopyMemoryToImageInfo *pCopyMemoryToImageInfo);
static VkResult __HINT_GCB_PFN_vkCopyImageToMemory(VkDevice device, const VkCopyImageToMemoryInfo *pCopyImageToMemoryInfo);
static VkResult __HINT_GCB_PFN_vkCopyImageToImage(VkDevice device, const VkCopyImageToImageInfo *pCopyImageToImageInfo);
static VkResult __HINT_GCB_PFN_vkTransitionImageLayout(VkDevice device, uint32_t transitionCount, const VkHostImageLayoutTransitionInfo *pTransitions);
static void __HINT_GCB_PFN_vkDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_GCB_PFN_vkGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32 *pSupported);
static VkResult __HINT_GCB_PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR *pSurfaceCapabilities);
static VkResult __HINT_GCB_PFN_vkGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t *pSurfaceFormatCount, VkSurfaceFormatKHR *pSurfaceFormats);
static VkResult __HINT_GCB_PFN_vkGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t *pPresentModeCount, VkPresentModeKHR *pPresentModes);
static VkResult __HINT_GCB_PFN_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchain);
static void __HINT_GCB_PFN_vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_GCB_PFN_vkGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t *pSwapchainImageCount, VkImage *pSwapchainImages);
static VkResult __HINT_GCB_PFN_vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex);
static VkResult __HINT_GCB_PFN_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo);
static VkResult __HINT_GCB_PFN_vkGetDeviceGroupPresentCapabilitiesKHR(VkDevice device, VkDeviceGroupPresentCapabilitiesKHR *pDeviceGroupPresentCapabilities);
static VkResult __HINT_GCB_PFN_vkGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface, VkDeviceGroupPresentModeFlagsKHR *pModes);
static VkResult __HINT_GCB_PFN_vkGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t *pRectCount, VkRect2D *pRects);
static VkResult __HINT_GCB_PFN_vkAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR *pAcquireInfo, uint32_t *pImageIndex);
static VkResult __HINT_GCB_PFN_vkGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount, VkDisplayPropertiesKHR *pProperties);
static VkResult __HINT_GCB_PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount, VkDisplayPlanePropertiesKHR *pProperties);
static VkResult __HINT_GCB_PFN_vkGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex, uint32_t *pDisplayCount, VkDisplayKHR *pDisplays);
static VkResult __HINT_GCB_PFN_vkGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t *pPropertyCount, VkDisplayModePropertiesKHR *pProperties);
static VkResult __HINT_GCB_PFN_vkCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, const VkDisplayModeCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDisplayModeKHR *pMode);
static VkResult __HINT_GCB_PFN_vkGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR *pCapabilities);
static VkResult __HINT_GCB_PFN_vkCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface);
static VkResult __HINT_GCB_PFN_vkCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount, const VkSwapchainCreateInfoKHR *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchains);
static VkResult __HINT_GCB_PFN_vkGetPhysicalDeviceVideoCapabilitiesKHR(VkPhysicalDevice physicalDevice, const VkVideoProfileInfoKHR *pVideoProfile, VkVideoCapabilitiesKHR *pCapabilities);
static VkResult __HINT_GCB_PFN_vkGetPhysicalDeviceVideoFormatPropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoFormatInfoKHR *pVideoFormatInfo, uint32_t *pVideoFormatPropertyCount, VkVideoFormatPropertiesKHR *pVideoFormatProperties);
static VkResult __HINT_GCB_PFN_vkCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkVideoSessionKHR *pVideoSession);
static void __HINT_GCB_PFN_vkDestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_GCB_PFN_vkGetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession, uint32_t *pMemoryRequirementsCount, VkVideoSessionMemoryRequirementsKHR *pMemoryRequirements);
static VkResult __HINT_GCB_PFN_vkBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession, uint32_t bindSessionMemoryInfoCount, const VkBindVideoSessionMemoryInfoKHR *pBindSessionMemoryInfos);
static VkResult __HINT_GCB_PFN_vkCreateVideoSessionParametersKHR(VkDevice device, const VkVideoSessionParametersCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkVideoSessionParametersKHR *pVideoSessionParameters);
static VkResult __HINT_GCB_PFN_vkUpdateVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters, const VkVideoSessionParametersUpdateInfoKHR *pUpdateInfo);
static void __HINT_GCB_PFN_vkDestroyVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters, const VkAllocationCallbacks *pAllocator);
static void __HINT_GCB_PFN_vkCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR *pBeginInfo);
static void __HINT_GCB_PFN_vkCmdEndVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoEndCodingInfoKHR *pEndCodingInfo);
static void __HINT_GCB_PFN_vkCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoCodingControlInfoKHR *pCodingControlInfo);
static void __HINT_GCB_PFN_vkCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR *pDecodeInfo);
static void __HINT_GCB_PFN_vkCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo);
static void __HINT_GCB_PFN_vkCmdEndRenderingKHR(VkCommandBuffer commandBuffer);
static void __HINT_GCB_PFN_vkGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2 *pFeatures);
static void __HINT_GCB_PFN_vkGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2 *pProperties);
static void __HINT_GCB_PFN_vkGetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2 *pFormatProperties);
static VkResult __HINT_GCB_PFN_vkGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo, VkImageFormatProperties2 *pImageFormatProperties);
static void __HINT_GCB_PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t *pQueueFamilyPropertyCount, VkQueueFamilyProperties2 *pQueueFamilyProperties);
static void __HINT_GCB_PFN_vkGetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2 *pMemoryProperties);
static void __HINT_GCB_PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2 *pFormatInfo, uint32_t *pPropertyCount, VkSparseImageFormatProperties2 *pProperties);
static void __HINT_GCB_PFN_vkGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags *pPeerMemoryFeatures);
static void __HINT_GCB_PFN_vkCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask);
static void __HINT_GCB_PFN_vkCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
static void __HINT_GCB_PFN_vkTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags);
static VkResult __HINT_GCB_PFN_vkEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t *pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroupProperties);
static void __HINT_GCB_PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo *pExternalBufferInfo, VkExternalBufferProperties *pExternalBufferProperties);
static VkResult __HINT_GCB_PFN_vkGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR *pGetFdInfo, int *pFd);
static VkResult __HINT_GCB_PFN_vkGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd, VkMemoryFdPropertiesKHR *pMemoryFdProperties);
static void __HINT_GCB_PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo *pExternalSemaphoreInfo, VkExternalSemaphoreProperties *pExternalSemaphoreProperties);
static VkResult __HINT_GCB_PFN_vkImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR *pImportSemaphoreFdInfo);
static VkResult __HINT_GCB_PFN_vkGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR *pGetFdInfo, int *pFd);
static void __HINT_GCB_PFN_vkCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount, const VkWriteDescriptorSet *pDescriptorWrites);
static void __HINT_GCB_PFN_vkCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer, VkDescriptorUpdateTemplate descriptorUpdateTemplate, VkPipelineLayout layout, uint32_t set, const void *pData);
static VkResult __HINT_GCB_PFN_vkCreateDescriptorUpdateTemplateKHR(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate);
static void __HINT_GCB_PFN_vkDestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks *pAllocator);
static void __HINT_GCB_PFN_vkUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void *pData);
static VkResult __HINT_GCB_PFN_vkCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass);
static void __HINT_GCB_PFN_vkCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin, const VkSubpassBeginInfo *pSubpassBeginInfo);
static void __HINT_GCB_PFN_vkCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo, const VkSubpassEndInfo *pSubpassEndInfo);
static void __HINT_GCB_PFN_vkCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo);
static VkResult __HINT_GCB_PFN_vkGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain);
static void __HINT_GCB_PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo *pExternalFenceInfo, VkExternalFenceProperties *pExternalFenceProperties);
static VkResult __HINT_GCB_PFN_vkImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR *pImportFenceFdInfo);
static VkResult __HINT_GCB_PFN_vkGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR *pGetFdInfo, int *pFd);
static VkResult __HINT_GCB_PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t *pCounterCount, VkPerformanceCounterKHR *pCounters, VkPerformanceCounterDescriptionKHR *pCounterDescriptions);
static void __HINT_GCB_PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(VkPhysicalDevice physicalDevice, const VkQueryPoolPerformanceCreateInfoKHR *pPerformanceQueryCreateInfo, uint32_t *pNumPasses);
static VkResult __HINT_GCB_PFN_vkAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR *pInfo);
static void __HINT_GCB_PFN_vkReleaseProfilingLockKHR(VkDevice device);
static VkResult __HINT_GCB_PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo, VkSurfaceCapabilities2KHR *pSurfaceCapabilities);
static VkResult __HINT_GCB_PFN_vkGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo, uint32_t *pSurfaceFormatCount, VkSurfaceFormat2KHR *pSurfaceFormats);
static VkResult __HINT_GCB_PFN_vkGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount, VkDisplayProperties2KHR *pProperties);
static VkResult __HINT_GCB_PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount, VkDisplayPlaneProperties2KHR *pProperties);
static VkResult __HINT_GCB_PFN_vkGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t *pPropertyCount, VkDisplayModeProperties2KHR *pProperties);
static VkResult __HINT_GCB_PFN_vkGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice, const VkDisplayPlaneInfo2KHR *pDisplayPlaneInfo, VkDisplayPlaneCapabilities2KHR *pCapabilities);
static void __HINT_GCB_PFN_vkGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2 *pInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __HINT_GCB_PFN_vkGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2 *pInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __HINT_GCB_PFN_vkGetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2 *pInfo, uint32_t *pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements);
static VkResult __HINT_GCB_PFN_vkCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSamplerYcbcrConversion *pYcbcrConversion);
static void __HINT_GCB_PFN_vkDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_GCB_PFN_vkBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo *pBindInfos);
static VkResult __HINT_GCB_PFN_vkBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo *pBindInfos);
static void __HINT_GCB_PFN_vkGetDescriptorSetLayoutSupportKHR(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo, VkDescriptorSetLayoutSupport *pSupport);
static void __HINT_GCB_PFN_vkCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);
static void __HINT_GCB_PFN_vkCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);
static VkResult __HINT_GCB_PFN_vkGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t *pValue);
static VkResult __HINT_GCB_PFN_vkWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout);
static VkResult __HINT_GCB_PFN_vkSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo);
static VkResult __HINT_GCB_PFN_vkGetPhysicalDeviceFragmentShadingRatesKHR(VkPhysicalDevice physicalDevice, uint32_t *pFragmentShadingRateCount, VkPhysicalDeviceFragmentShadingRateKHR *pFragmentShadingRates);
static void __HINT_GCB_PFN_vkCmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer, const VkExtent2D *pFragmentSize, const VkFragmentShadingRateCombinerOpKHR combinerOps[2]);
static void __HINT_GCB_PFN_vkCmdSetRenderingAttachmentLocationsKHR(VkCommandBuffer commandBuffer, const VkRenderingAttachmentLocationInfo *pLocationInfo);
static void __HINT_GCB_PFN_vkCmdSetRenderingInputAttachmentIndicesKHR(VkCommandBuffer commandBuffer, const VkRenderingInputAttachmentIndexInfo *pInputAttachmentIndexInfo);
static VkResult __HINT_GCB_PFN_vkWaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId, uint64_t timeout);
static VkDeviceAddress __HINT_GCB_PFN_vkGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo *pInfo);
static uint64_t __HINT_GCB_PFN_vkGetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo *pInfo);
static uint64_t __HINT_GCB_PFN_vkGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo *pInfo);
static VkResult __HINT_GCB_PFN_vkCreateDeferredOperationKHR(VkDevice device, const VkAllocationCallbacks *pAllocator, VkDeferredOperationKHR *pDeferredOperation);
static void __HINT_GCB_PFN_vkDestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation, const VkAllocationCallbacks *pAllocator);
static uint32_t __HINT_GCB_PFN_vkGetDeferredOperationMaxConcurrencyKHR(VkDevice device, VkDeferredOperationKHR operation);
static VkResult __HINT_GCB_PFN_vkGetDeferredOperationResultKHR(VkDevice device, VkDeferredOperationKHR operation);
static VkResult __HINT_GCB_PFN_vkDeferredOperationJoinKHR(VkDevice device, VkDeferredOperationKHR operation);
static VkResult __HINT_GCB_PFN_vkGetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR *pPipelineInfo, uint32_t *pExecutableCount, VkPipelineExecutablePropertiesKHR *pProperties);
static VkResult __HINT_GCB_PFN_vkGetPipelineExecutableStatisticsKHR(VkDevice device, const VkPipelineExecutableInfoKHR *pExecutableInfo, uint32_t *pStatisticCount, VkPipelineExecutableStatisticKHR *pStatistics);
static VkResult __HINT_GCB_PFN_vkGetPipelineExecutableInternalRepresentationsKHR(VkDevice device, const VkPipelineExecutableInfoKHR *pExecutableInfo, uint32_t *pInternalRepresentationCount, VkPipelineExecutableInternalRepresentationKHR *pInternalRepresentations);
static VkResult __HINT_GCB_PFN_vkMapMemory2KHR(VkDevice device, const VkMemoryMapInfo *pMemoryMapInfo, void **ppData);
static VkResult __HINT_GCB_PFN_vkUnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfo *pMemoryUnmapInfo);
static VkResult __HINT_GCB_PFN_vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR *pQualityLevelInfo, VkVideoEncodeQualityLevelPropertiesKHR *pQualityLevelProperties);
static VkResult __HINT_GCB_PFN_vkGetEncodedVideoSessionParametersKHR(VkDevice device, const VkVideoEncodeSessionParametersGetInfoKHR *pVideoSessionParametersInfo, VkVideoEncodeSessionParametersFeedbackInfoKHR *pFeedbackInfo, size_t *pDataSize, void *pData);
static void __HINT_GCB_PFN_vkCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR *pEncodeInfo);
static void __HINT_GCB_PFN_vkCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo *pDependencyInfo);
static void __HINT_GCB_PFN_vkCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask);
static void __HINT_GCB_PFN_vkCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents, const VkDependencyInfo *pDependencyInfos);
static void __HINT_GCB_PFN_vkCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo);
static void __HINT_GCB_PFN_vkCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool, uint32_t query);
static VkResult __HINT_GCB_PFN_vkQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits, VkFence fence);
static void __HINT_GCB_PFN_vkCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo);
static void __HINT_GCB_PFN_vkCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo);
static void __HINT_GCB_PFN_vkCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo);
static void __HINT_GCB_PFN_vkCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo);
static void __HINT_GCB_PFN_vkCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2 *pBlitImageInfo);
static void __HINT_GCB_PFN_vkCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2 *pResolveImageInfo);
static void __HINT_GCB_PFN_vkCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress);
static void __HINT_GCB_PFN_vkGetDeviceBufferMemoryRequirementsKHR(VkDevice device, const VkDeviceBufferMemoryRequirements *pInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __HINT_GCB_PFN_vkGetDeviceImageMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements *pInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __HINT_GCB_PFN_vkGetDeviceImageSparseMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements *pInfo, uint32_t *pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements);
static void __HINT_GCB_PFN_vkCmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size, VkIndexType indexType);
static void __HINT_GCB_PFN_vkGetRenderingAreaGranularityKHR(VkDevice device, const VkRenderingAreaInfo *pRenderingAreaInfo, VkExtent2D *pGranularity);
static void __HINT_GCB_PFN_vkGetDeviceImageSubresourceLayoutKHR(VkDevice device, const VkDeviceImageSubresourceInfo *pInfo, VkSubresourceLayout2 *pLayout);
static void __HINT_GCB_PFN_vkGetImageSubresourceLayout2KHR(VkDevice device, VkImage image, const VkImageSubresource2 *pSubresource, VkSubresourceLayout2 *pLayout);
static VkResult __HINT_GCB_PFN_vkWaitForPresent2KHR(VkDevice device, VkSwapchainKHR swapchain, const VkPresentWait2InfoKHR *pPresentWait2Info);
static VkResult __HINT_GCB_PFN_vkCreatePipelineBinariesKHR(VkDevice device, const VkPipelineBinaryCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkPipelineBinaryHandlesInfoKHR *pBinaries);
static void __HINT_GCB_PFN_vkDestroyPipelineBinaryKHR(VkDevice device, VkPipelineBinaryKHR pipelineBinary, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_GCB_PFN_vkGetPipelineKeyKHR(VkDevice device, const VkPipelineCreateInfoKHR *pPipelineCreateInfo, VkPipelineBinaryKeyKHR *pPipelineKey);
static VkResult __HINT_GCB_PFN_vkGetPipelineBinaryDataKHR(VkDevice device, const VkPipelineBinaryDataInfoKHR *pInfo, VkPipelineBinaryKeyKHR *pPipelineBinaryKey, size_t *pPipelineBinaryDataSize, void *pPipelineBinaryData);
static VkResult __HINT_GCB_PFN_vkReleaseCapturedPipelineDataKHR(VkDevice device, const VkReleaseCapturedPipelineDataInfoKHR *pInfo, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_GCB_PFN_vkReleaseSwapchainImagesKHR(VkDevice device, const VkReleaseSwapchainImagesInfoKHR *pReleaseInfo);
static VkResult __HINT_GCB_PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount, VkCooperativeMatrixPropertiesKHR *pProperties);
static void __HINT_GCB_PFN_vkCmdSetLineStippleKHR(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern);
static VkResult __HINT_GCB_PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsKHR(VkPhysicalDevice physicalDevice, uint32_t *pTimeDomainCount, VkTimeDomainKHR *pTimeDomains);
static VkResult __HINT_GCB_PFN_vkGetCalibratedTimestampsKHR(VkDevice device, uint32_t timestampCount, const VkCalibratedTimestampInfoKHR *pTimestampInfos, uint64_t *pTimestamps, uint64_t *pMaxDeviation);
static void __HINT_GCB_PFN_vkCmdBindDescriptorSets2KHR(VkCommandBuffer commandBuffer, const VkBindDescriptorSetsInfo *pBindDescriptorSetsInfo);
static void __HINT_GCB_PFN_vkCmdPushConstants2KHR(VkCommandBuffer commandBuffer, const VkPushConstantsInfo *pPushConstantsInfo);
static void __HINT_GCB_PFN_vkCmdPushDescriptorSet2KHR(VkCommandBuffer commandBuffer, const VkPushDescriptorSetInfo *pPushDescriptorSetInfo);
static void __HINT_GCB_PFN_vkCmdPushDescriptorSetWithTemplate2KHR(VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfo *pPushDescriptorSetWithTemplateInfo);
static void __HINT_GCB_PFN_vkCmdSetDescriptorBufferOffsets2EXT(VkCommandBuffer commandBuffer, const VkSetDescriptorBufferOffsetsInfoEXT *pSetDescriptorBufferOffsetsInfo);
static void __HINT_GCB_PFN_vkCmdBindDescriptorBufferEmbeddedSamplers2EXT(VkCommandBuffer commandBuffer, const VkBindDescriptorBufferEmbeddedSamplersInfoEXT *pBindDescriptorBufferEmbeddedSamplersInfo);
static VkResult __HINT_GCB_PFN_vkCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugReportCallbackEXT *pCallback);
static void __HINT_GCB_PFN_vkDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks *pAllocator);
static void __HINT_GCB_PFN_vkDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char *pLayerPrefix, const char *pMessage);
static VkResult __HINT_GCB_PFN_vkDebugMarkerSetObjectTagEXT(VkDevice device, const VkDebugMarkerObjectTagInfoEXT *pTagInfo);
static VkResult __HINT_GCB_PFN_vkDebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT *pNameInfo);
static void __HINT_GCB_PFN_vkCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT *pMarkerInfo);
static void __HINT_GCB_PFN_vkCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer);
static void __HINT_GCB_PFN_vkCmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT *pMarkerInfo);
static void __HINT_GCB_PFN_vkCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer *pBuffers, const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes);
static void __HINT_GCB_PFN_vkCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer, uint32_t counterBufferCount, const VkBuffer *pCounterBuffers, const VkDeviceSize *pCounterBufferOffsets);
static void __HINT_GCB_PFN_vkCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer, uint32_t counterBufferCount, const VkBuffer *pCounterBuffers, const VkDeviceSize *pCounterBufferOffsets);
static void __HINT_GCB_PFN_vkCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags, uint32_t index);
static void __HINT_GCB_PFN_vkCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, uint32_t index);
static void __HINT_GCB_PFN_vkCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance, VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset, uint32_t vertexStride);
static VkResult __HINT_GCB_PFN_vkCreateCuModuleNVX(VkDevice device, const VkCuModuleCreateInfoNVX *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkCuModuleNVX *pModule);
static VkResult __HINT_GCB_PFN_vkCreateCuFunctionNVX(VkDevice device, const VkCuFunctionCreateInfoNVX *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkCuFunctionNVX *pFunction);
static void __HINT_GCB_PFN_vkDestroyCuModuleNVX(VkDevice device, VkCuModuleNVX module, const VkAllocationCallbacks *pAllocator);
static void __HINT_GCB_PFN_vkDestroyCuFunctionNVX(VkDevice device, VkCuFunctionNVX function, const VkAllocationCallbacks *pAllocator);
static void __HINT_GCB_PFN_vkCmdCuLaunchKernelNVX(VkCommandBuffer commandBuffer, const VkCuLaunchInfoNVX *pLaunchInfo);
static uint32_t __HINT_GCB_PFN_vkGetImageViewHandleNVX(VkDevice device, const VkImageViewHandleInfoNVX *pInfo);
static uint64_t __HINT_GCB_PFN_vkGetImageViewHandle64NVX(VkDevice device, const VkImageViewHandleInfoNVX *pInfo);
static VkResult __HINT_GCB_PFN_vkGetImageViewAddressNVX(VkDevice device, VkImageView imageView, VkImageViewAddressPropertiesNVX *pProperties);
static void __HINT_GCB_PFN_vkCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);
static void __HINT_GCB_PFN_vkCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);
static VkResult __HINT_GCB_PFN_vkGetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage, VkShaderInfoTypeAMD infoType, size_t *pInfoSize, void *pInfo);
static VkResult __HINT_GCB_PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType, VkExternalImageFormatPropertiesNV *pExternalImageFormatProperties);
static void __HINT_GCB_PFN_vkCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfoEXT *pConditionalRenderingBegin);
static void __HINT_GCB_PFN_vkCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer);
static void __HINT_GCB_PFN_vkCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewportWScalingNV *pViewportWScalings);
static VkResult __HINT_GCB_PFN_vkReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display);
static VkResult __HINT_GCB_PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilities2EXT *pSurfaceCapabilities);
static VkResult __HINT_GCB_PFN_vkDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display, const VkDisplayPowerInfoEXT *pDisplayPowerInfo);
static VkResult __HINT_GCB_PFN_vkRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT *pDeviceEventInfo, const VkAllocationCallbacks *pAllocator, VkFence *pFence);
static VkResult __HINT_GCB_PFN_vkRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display, const VkDisplayEventInfoEXT *pDisplayEventInfo, const VkAllocationCallbacks *pAllocator, VkFence *pFence);
static VkResult __HINT_GCB_PFN_vkGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter, uint64_t *pCounterValue);
static VkResult __HINT_GCB_PFN_vkGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain, VkRefreshCycleDurationGOOGLE *pDisplayTimingProperties);
static VkResult __HINT_GCB_PFN_vkGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain, uint32_t *pPresentationTimingCount, VkPastPresentationTimingGOOGLE *pPresentationTimings);
static void __HINT_GCB_PFN_vkCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle, uint32_t discardRectangleCount, const VkRect2D *pDiscardRectangles);
static void __HINT_GCB_PFN_vkCmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 discardRectangleEnable);
static void __HINT_GCB_PFN_vkCmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer, VkDiscardRectangleModeEXT discardRectangleMode);
static void __HINT_GCB_PFN_vkSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR *pSwapchains, const VkHdrMetadataEXT *pMetadata);
static VkResult __HINT_GCB_PFN_vkSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT *pNameInfo);
static VkResult __HINT_GCB_PFN_vkSetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT *pTagInfo);
static void __HINT_GCB_PFN_vkQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT *pLabelInfo);
static void __HINT_GCB_PFN_vkQueueEndDebugUtilsLabelEXT(VkQueue queue);
static void __HINT_GCB_PFN_vkQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT *pLabelInfo);
static void __HINT_GCB_PFN_vkCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT *pLabelInfo);
static void __HINT_GCB_PFN_vkCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer);
static void __HINT_GCB_PFN_vkCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT *pLabelInfo);
static VkResult __HINT_GCB_PFN_vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pMessenger);
static void __HINT_GCB_PFN_vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks *pAllocator);
static void __HINT_GCB_PFN_vkSubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData);
static void __HINT_GCB_PFN_vkCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer, const VkSampleLocationsInfoEXT *pSampleLocationsInfo);
static void __HINT_GCB_PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice physicalDevice, VkSampleCountFlagBits samples, VkMultisamplePropertiesEXT *pMultisampleProperties);
static VkResult __HINT_GCB_PFN_vkGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image, VkImageDrmFormatModifierPropertiesEXT *pProperties);
static VkResult __HINT_GCB_PFN_vkCreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkValidationCacheEXT *pValidationCache);
static void __HINT_GCB_PFN_vkDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_GCB_PFN_vkMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount, const VkValidationCacheEXT *pSrcCaches);
static VkResult __HINT_GCB_PFN_vkGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t *pDataSize, void *pData);
static void __HINT_GCB_PFN_vkCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout);
static void __HINT_GCB_PFN_vkCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkShadingRatePaletteNV *pShadingRatePalettes);
static void __HINT_GCB_PFN_vkCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType, uint32_t customSampleOrderCount, const VkCoarseSampleOrderCustomNV *pCustomSampleOrders);
static VkResult __HINT_GCB_PFN_vkCreateAccelerationStructureNV(VkDevice device, const VkAccelerationStructureCreateInfoNV *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkAccelerationStructureNV *pAccelerationStructure);
static void __HINT_GCB_PFN_vkDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure, const VkAllocationCallbacks *pAllocator);
static void __HINT_GCB_PFN_vkGetAccelerationStructureMemoryRequirementsNV(VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoNV *pInfo, VkMemoryRequirements2KHR *pMemoryRequirements);
static VkResult __HINT_GCB_PFN_vkBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount, const VkBindAccelerationStructureMemoryInfoNV *pBindInfos);
static void __HINT_GCB_PFN_vkCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV *pInfo, VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update, VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkBuffer scratch, VkDeviceSize scratchOffset);
static void __HINT_GCB_PFN_vkCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkCopyAccelerationStructureModeKHR mode);
static void __HINT_GCB_PFN_vkCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer, VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer, VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride, VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset, VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer, VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride, uint32_t width, uint32_t height, uint32_t depth);
static VkResult __HINT_GCB_PFN_vkCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoNV *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines);
static VkResult __HINT_GCB_PFN_vkGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void *pData);
static VkResult __HINT_GCB_PFN_vkGetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void *pData);
static VkResult __HINT_GCB_PFN_vkGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure, size_t dataSize, void *pData);
static void __HINT_GCB_PFN_vkCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureNV *pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery);
static VkResult __HINT_GCB_PFN_vkCompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader);
static VkResult __HINT_GCB_PFN_vkGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, const void *pHostPointer, VkMemoryHostPointerPropertiesEXT *pMemoryHostPointerProperties);
static void __HINT_GCB_PFN_vkCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker);
static void __HINT_GCB_PFN_vkCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker);
static VkResult __HINT_GCB_PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT(VkPhysicalDevice physicalDevice, uint32_t *pTimeDomainCount, VkTimeDomainKHR *pTimeDomains);
static VkResult __HINT_GCB_PFN_vkGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount, const VkCalibratedTimestampInfoKHR *pTimestampInfos, uint64_t *pTimestamps, uint64_t *pMaxDeviation);
static void __HINT_GCB_PFN_vkCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask);
static void __HINT_GCB_PFN_vkCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
static void __HINT_GCB_PFN_vkCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);
static void __HINT_GCB_PFN_vkCmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor, uint32_t exclusiveScissorCount, const VkBool32 *pExclusiveScissorEnables);
static void __HINT_GCB_PFN_vkCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor, uint32_t exclusiveScissorCount, const VkRect2D *pExclusiveScissors);
static void __HINT_GCB_PFN_vkCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void *pCheckpointMarker);
static void __HINT_GCB_PFN_vkGetQueueCheckpointDataNV(VkQueue queue, uint32_t *pCheckpointDataCount, VkCheckpointDataNV *pCheckpointData);
static void __HINT_GCB_PFN_vkGetQueueCheckpointData2NV(VkQueue queue, uint32_t *pCheckpointDataCount, VkCheckpointData2NV *pCheckpointData);
static VkResult __HINT_GCB_PFN_vkInitializePerformanceApiINTEL(VkDevice device, const VkInitializePerformanceApiInfoINTEL *pInitializeInfo);
static void __HINT_GCB_PFN_vkUninitializePerformanceApiINTEL(VkDevice device);
static VkResult __HINT_GCB_PFN_vkCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer, const VkPerformanceMarkerInfoINTEL *pMarkerInfo);
static VkResult __HINT_GCB_PFN_vkCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer, const VkPerformanceStreamMarkerInfoINTEL *pMarkerInfo);
static VkResult __HINT_GCB_PFN_vkCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer, const VkPerformanceOverrideInfoINTEL *pOverrideInfo);
static VkResult __HINT_GCB_PFN_vkAcquirePerformanceConfigurationINTEL(VkDevice device, const VkPerformanceConfigurationAcquireInfoINTEL *pAcquireInfo, VkPerformanceConfigurationINTEL *pConfiguration);
static VkResult __HINT_GCB_PFN_vkReleasePerformanceConfigurationINTEL(VkDevice device, VkPerformanceConfigurationINTEL configuration);
static VkResult __HINT_GCB_PFN_vkQueueSetPerformanceConfigurationINTEL(VkQueue queue, VkPerformanceConfigurationINTEL configuration);
static VkResult __HINT_GCB_PFN_vkGetPerformanceParameterINTEL(VkDevice device, VkPerformanceParameterTypeINTEL parameter, VkPerformanceValueINTEL *pValue);
static void __HINT_GCB_PFN_vkSetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapChain, VkBool32 localDimmingEnable);
static VkDeviceAddress __HINT_GCB_PFN_vkGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo *pInfo);
static VkResult __HINT_GCB_PFN_vkGetPhysicalDeviceToolPropertiesEXT(VkPhysicalDevice physicalDevice, uint32_t *pToolCount, VkPhysicalDeviceToolProperties *pToolProperties);
static VkResult __HINT_GCB_PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount, VkCooperativeMatrixPropertiesNV *pProperties);
static VkResult __HINT_GCB_PFN_vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(VkPhysicalDevice physicalDevice, uint32_t *pCombinationCount, VkFramebufferMixedSamplesCombinationNV *pCombinations);
static VkResult __HINT_GCB_PFN_vkCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface);
static void __HINT_GCB_PFN_vkCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern);
static void __HINT_GCB_PFN_vkResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);
static void __HINT_GCB_PFN_vkCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode);
static void __HINT_GCB_PFN_vkCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace);
static void __HINT_GCB_PFN_vkCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology);
static void __HINT_GCB_PFN_vkCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport *pViewports);
static void __HINT_GCB_PFN_vkCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D *pScissors);
static void __HINT_GCB_PFN_vkCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer *pBuffers, const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes, const VkDeviceSize *pStrides);
static void __HINT_GCB_PFN_vkCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable);
static void __HINT_GCB_PFN_vkCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable);
static void __HINT_GCB_PFN_vkCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp);
static void __HINT_GCB_PFN_vkCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable);
static void __HINT_GCB_PFN_vkCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable);
static void __HINT_GCB_PFN_vkCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp);
static VkResult __HINT_GCB_PFN_vkCopyMemoryToImageEXT(VkDevice device, const VkCopyMemoryToImageInfo *pCopyMemoryToImageInfo);
static VkResult __HINT_GCB_PFN_vkCopyImageToMemoryEXT(VkDevice device, const VkCopyImageToMemoryInfo *pCopyImageToMemoryInfo);
static VkResult __HINT_GCB_PFN_vkCopyImageToImageEXT(VkDevice device, const VkCopyImageToImageInfo *pCopyImageToImageInfo);
static VkResult __HINT_GCB_PFN_vkTransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount, const VkHostImageLayoutTransitionInfo *pTransitions);
static void __HINT_GCB_PFN_vkGetImageSubresourceLayout2EXT(VkDevice device, VkImage image, const VkImageSubresource2 *pSubresource, VkSubresourceLayout2 *pLayout);
static VkResult __HINT_GCB_PFN_vkReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoKHR *pReleaseInfo);
static void __HINT_GCB_PFN_vkGetGeneratedCommandsMemoryRequirementsNV(VkDevice device, const VkGeneratedCommandsMemoryRequirementsInfoNV *pInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __HINT_GCB_PFN_vkCmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer, const VkGeneratedCommandsInfoNV *pGeneratedCommandsInfo);
static void __HINT_GCB_PFN_vkCmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed, const VkGeneratedCommandsInfoNV *pGeneratedCommandsInfo);
static void __HINT_GCB_PFN_vkCmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline, uint32_t groupIndex);
static VkResult __HINT_GCB_PFN_vkCreateIndirectCommandsLayoutNV(VkDevice device, const VkIndirectCommandsLayoutCreateInfoNV *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkIndirectCommandsLayoutNV *pIndirectCommandsLayout);
static void __HINT_GCB_PFN_vkDestroyIndirectCommandsLayoutNV(VkDevice device, VkIndirectCommandsLayoutNV indirectCommandsLayout, const VkAllocationCallbacks *pAllocator);
static void __HINT_GCB_PFN_vkCmdSetDepthBias2EXT(VkCommandBuffer commandBuffer, const VkDepthBiasInfoEXT *pDepthBiasInfo);
static VkResult __HINT_GCB_PFN_vkAcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, VkDisplayKHR display);
static VkResult __HINT_GCB_PFN_vkGetDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, uint32_t connectorId, VkDisplayKHR *display);
static VkResult __HINT_GCB_PFN_vkCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkPrivateDataSlot *pPrivateDataSlot);
static void __HINT_GCB_PFN_vkDestroyPrivateDataSlotEXT(VkDevice device, VkPrivateDataSlot privateDataSlot, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_GCB_PFN_vkSetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlot privateDataSlot, uint64_t data);
static void __HINT_GCB_PFN_vkGetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlot privateDataSlot, uint64_t *pData);
static void __HINT_GCB_PFN_vkCmdDispatchTileQCOM(VkCommandBuffer commandBuffer, const VkDispatchTileInfoQCOM *pDispatchTileInfo);
static void __HINT_GCB_PFN_vkCmdBeginPerTileExecutionQCOM(VkCommandBuffer commandBuffer, const VkPerTileBeginInfoQCOM *pPerTileBeginInfo);
static void __HINT_GCB_PFN_vkCmdEndPerTileExecutionQCOM(VkCommandBuffer commandBuffer, const VkPerTileEndInfoQCOM *pPerTileEndInfo);
static void __HINT_GCB_PFN_vkGetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout, VkDeviceSize *pLayoutSizeInBytes);
static void __HINT_GCB_PFN_vkGetDescriptorSetLayoutBindingOffsetEXT(VkDevice device, VkDescriptorSetLayout layout, uint32_t binding, VkDeviceSize *pOffset);
static void __HINT_GCB_PFN_vkGetDescriptorEXT(VkDevice device, const VkDescriptorGetInfoEXT *pDescriptorInfo, size_t dataSize, void *pDescriptor);
static void __HINT_GCB_PFN_vkCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount, const VkDescriptorBufferBindingInfoEXT *pBindingInfos);
static void __HINT_GCB_PFN_vkCmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount, const uint32_t *pBufferIndices, const VkDeviceSize *pOffsets);
static void __HINT_GCB_PFN_vkCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t set);
static VkResult __HINT_GCB_PFN_vkGetBufferOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkBufferCaptureDescriptorDataInfoEXT *pInfo, void *pData);
static VkResult __HINT_GCB_PFN_vkGetImageOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkImageCaptureDescriptorDataInfoEXT *pInfo, void *pData);
static VkResult __HINT_GCB_PFN_vkGetImageViewOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkImageViewCaptureDescriptorDataInfoEXT *pInfo, void *pData);
static VkResult __HINT_GCB_PFN_vkGetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkSamplerCaptureDescriptorDataInfoEXT *pInfo, void *pData);
static VkResult __HINT_GCB_PFN_vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkAccelerationStructureCaptureDescriptorDataInfoEXT *pInfo, void *pData);
static void __HINT_GCB_PFN_vkCmdSetFragmentShadingRateEnumNV(VkCommandBuffer commandBuffer, VkFragmentShadingRateNV shadingRate, const VkFragmentShadingRateCombinerOpKHR combinerOps[2]);
static VkResult __HINT_GCB_PFN_vkGetDeviceFaultInfoEXT(VkDevice device, VkDeviceFaultCountsEXT *pFaultCounts, VkDeviceFaultInfoEXT *pFaultInfo);
static void __HINT_GCB_PFN_vkCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount, const VkVertexInputBindingDescription2EXT *pVertexBindingDescriptions, uint32_t vertexAttributeDescriptionCount, const VkVertexInputAttributeDescription2EXT *pVertexAttributeDescriptions);
static VkResult __HINT_GCB_PFN_vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(VkDevice device, VkRenderPass renderpass, VkExtent2D *pMaxWorkgroupSize);
static void __HINT_GCB_PFN_vkCmdSubpassShadingHUAWEI(VkCommandBuffer commandBuffer);
static void __HINT_GCB_PFN_vkCmdBindInvocationMaskHUAWEI(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout);
static VkResult __HINT_GCB_PFN_vkGetMemoryRemoteAddressNV(VkDevice device, const VkMemoryGetRemoteAddressInfoNV *pMemoryGetRemoteAddressInfo, VkRemoteAddressNV *pAddress);
static VkResult __HINT_GCB_PFN_vkGetPipelinePropertiesEXT(VkDevice device, const VkPipelineInfoEXT *pPipelineInfo, VkBaseOutStructure *pPipelineProperties);
static void __HINT_GCB_PFN_vkCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints);
static void __HINT_GCB_PFN_vkCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable);
static void __HINT_GCB_PFN_vkCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable);
static void __HINT_GCB_PFN_vkCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp);
static void __HINT_GCB_PFN_vkCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable);
static void __HINT_GCB_PFN_vkCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkBool32 *pColorWriteEnables);
static void __HINT_GCB_PFN_vkCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT *pVertexInfo, uint32_t instanceCount, uint32_t firstInstance, uint32_t stride);
static void __HINT_GCB_PFN_vkCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawIndexedInfoEXT *pIndexInfo, uint32_t instanceCount, uint32_t firstInstance, uint32_t stride, const int32_t *pVertexOffset);
static VkResult __HINT_GCB_PFN_vkCreateMicromapEXT(VkDevice device, const VkMicromapCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkMicromapEXT *pMicromap);
static void __HINT_GCB_PFN_vkDestroyMicromapEXT(VkDevice device, VkMicromapEXT micromap, const VkAllocationCallbacks *pAllocator);
static void __HINT_GCB_PFN_vkCmdBuildMicromapsEXT(VkCommandBuffer commandBuffer, uint32_t infoCount, const VkMicromapBuildInfoEXT *pInfos);
static VkResult __HINT_GCB_PFN_vkBuildMicromapsEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount, const VkMicromapBuildInfoEXT *pInfos);
static VkResult __HINT_GCB_PFN_vkCopyMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyMicromapInfoEXT *pInfo);
static VkResult __HINT_GCB_PFN_vkCopyMicromapToMemoryEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyMicromapToMemoryInfoEXT *pInfo);
static VkResult __HINT_GCB_PFN_vkCopyMemoryToMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyMemoryToMicromapInfoEXT *pInfo);
static VkResult __HINT_GCB_PFN_vkWriteMicromapsPropertiesEXT(VkDevice device, uint32_t micromapCount, const VkMicromapEXT *pMicromaps, VkQueryType queryType, size_t dataSize, void *pData, size_t stride);
static void __HINT_GCB_PFN_vkCmdCopyMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapInfoEXT *pInfo);
static void __HINT_GCB_PFN_vkCmdCopyMicromapToMemoryEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapToMemoryInfoEXT *pInfo);
static void __HINT_GCB_PFN_vkCmdCopyMemoryToMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMemoryToMicromapInfoEXT *pInfo);
static void __HINT_GCB_PFN_vkCmdWriteMicromapsPropertiesEXT(VkCommandBuffer commandBuffer, uint32_t micromapCount, const VkMicromapEXT *pMicromaps, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery);
static void __HINT_GCB_PFN_vkGetDeviceMicromapCompatibilityEXT(VkDevice device, const VkMicromapVersionInfoEXT *pVersionInfo, VkAccelerationStructureCompatibilityKHR *pCompatibility);
static void __HINT_GCB_PFN_vkGetMicromapBuildSizesEXT(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType, const VkMicromapBuildInfoEXT *pBuildInfo, VkMicromapBuildSizesInfoEXT *pSizeInfo);
static void __HINT_GCB_PFN_vkCmdDrawClusterHUAWEI(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
static void __HINT_GCB_PFN_vkCmdDrawClusterIndirectHUAWEI(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset);
static void __HINT_GCB_PFN_vkSetDeviceMemoryPriorityEXT(VkDevice device, VkDeviceMemory memory, float priority);
static void __HINT_GCB_PFN_vkGetDescriptorSetLayoutHostMappingInfoVALVE(VkDevice device, const VkDescriptorSetBindingReferenceVALVE *pBindingReference, VkDescriptorSetLayoutHostMappingInfoVALVE *pHostMapping);
static void __HINT_GCB_PFN_vkGetDescriptorSetHostMappingVALVE(VkDevice device, VkDescriptorSet descriptorSet, void **ppData);
static void __HINT_GCB_PFN_vkCmdCopyMemoryIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress, uint32_t copyCount, uint32_t stride);
static void __HINT_GCB_PFN_vkCmdCopyMemoryToImageIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress, uint32_t copyCount, uint32_t stride, VkImage dstImage, VkImageLayout dstImageLayout, const VkImageSubresourceLayers *pImageSubresources);
static void __HINT_GCB_PFN_vkCmdDecompressMemoryNV(VkCommandBuffer commandBuffer, uint32_t decompressRegionCount, const VkDecompressMemoryRegionNV *pDecompressMemoryRegions);
static void __HINT_GCB_PFN_vkCmdDecompressMemoryIndirectCountNV(VkCommandBuffer commandBuffer, VkDeviceAddress indirectCommandsAddress, VkDeviceAddress indirectCommandsCountAddress, uint32_t stride);
static void __HINT_GCB_PFN_vkGetPipelineIndirectMemoryRequirementsNV(VkDevice device, const VkComputePipelineCreateInfo *pCreateInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __HINT_GCB_PFN_vkCmdUpdatePipelineIndirectBufferNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);
static VkDeviceAddress __HINT_GCB_PFN_vkGetPipelineIndirectDeviceAddressNV(VkDevice device, const VkPipelineIndirectDeviceAddressInfoNV *pInfo);
static void __HINT_GCB_PFN_vkCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable);
static void __HINT_GCB_PFN_vkCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode);
static void __HINT_GCB_PFN_vkCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits rasterizationSamples);
static void __HINT_GCB_PFN_vkCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples, const VkSampleMask *pSampleMask);
static void __HINT_GCB_PFN_vkCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToCoverageEnable);
static void __HINT_GCB_PFN_vkCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable);
static void __HINT_GCB_PFN_vkCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable);
static void __HINT_GCB_PFN_vkCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount, const VkBool32 *pColorBlendEnables);
static void __HINT_GCB_PFN_vkCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount, const VkColorBlendEquationEXT *pColorBlendEquations);
static void __HINT_GCB_PFN_vkCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount, const VkColorComponentFlags *pColorWriteMasks);
static void __HINT_GCB_PFN_vkCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer, VkTessellationDomainOrigin domainOrigin);
static void __HINT_GCB_PFN_vkCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream);
static void __HINT_GCB_PFN_vkCmdSetConservativeRasterizationModeEXT(VkCommandBuffer commandBuffer, VkConservativeRasterizationModeEXT conservativeRasterizationMode);
static void __HINT_GCB_PFN_vkCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer, float extraPrimitiveOverestimationSize);
static void __HINT_GCB_PFN_vkCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable);
static void __HINT_GCB_PFN_vkCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer, VkBool32 sampleLocationsEnable);
static void __HINT_GCB_PFN_vkCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount, const VkColorBlendAdvancedEXT *pColorBlendAdvanced);
static void __HINT_GCB_PFN_vkCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer, VkProvokingVertexModeEXT provokingVertexMode);
static void __HINT_GCB_PFN_vkCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer, VkLineRasterizationModeEXT lineRasterizationMode);
static void __HINT_GCB_PFN_vkCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable);
static void __HINT_GCB_PFN_vkCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne);
static void __HINT_GCB_PFN_vkCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer, VkBool32 viewportWScalingEnable);
static void __HINT_GCB_PFN_vkCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewportSwizzleNV *pViewportSwizzles);
static void __HINT_GCB_PFN_vkCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageToColorEnable);
static void __HINT_GCB_PFN_vkCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer, uint32_t coverageToColorLocation);
static void __HINT_GCB_PFN_vkCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer, VkCoverageModulationModeNV coverageModulationMode);
static void __HINT_GCB_PFN_vkCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageModulationTableEnable);
static void __HINT_GCB_PFN_vkCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer, uint32_t coverageModulationTableCount, const float *pCoverageModulationTable);
static void __HINT_GCB_PFN_vkCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer, VkBool32 shadingRateImageEnable);
static void __HINT_GCB_PFN_vkCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer, VkBool32 representativeFragmentTestEnable);
static void __HINT_GCB_PFN_vkCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer, VkCoverageReductionModeNV coverageReductionMode);
static void __HINT_GCB_PFN_vkGetShaderModuleIdentifierEXT(VkDevice device, VkShaderModule shaderModule, VkShaderModuleIdentifierEXT *pIdentifier);
static void __HINT_GCB_PFN_vkGetShaderModuleCreateInfoIdentifierEXT(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo, VkShaderModuleIdentifierEXT *pIdentifier);
static VkResult __HINT_GCB_PFN_vkGetPhysicalDeviceOpticalFlowImageFormatsNV(VkPhysicalDevice physicalDevice, const VkOpticalFlowImageFormatInfoNV *pOpticalFlowImageFormatInfo, uint32_t *pFormatCount, VkOpticalFlowImageFormatPropertiesNV *pImageFormatProperties);
static VkResult __HINT_GCB_PFN_vkCreateOpticalFlowSessionNV(VkDevice device, const VkOpticalFlowSessionCreateInfoNV *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkOpticalFlowSessionNV *pSession);
static void __HINT_GCB_PFN_vkDestroyOpticalFlowSessionNV(VkDevice device, VkOpticalFlowSessionNV session, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_GCB_PFN_vkBindOpticalFlowSessionImageNV(VkDevice device, VkOpticalFlowSessionNV session, VkOpticalFlowSessionBindingPointNV bindingPoint, VkImageView view, VkImageLayout layout);
static void __HINT_GCB_PFN_vkCmdOpticalFlowExecuteNV(VkCommandBuffer commandBuffer, VkOpticalFlowSessionNV session, const VkOpticalFlowExecuteInfoNV *pExecuteInfo);
static void __HINT_GCB_PFN_vkAntiLagUpdateAMD(VkDevice device, const VkAntiLagDataAMD *pData);
static VkResult __HINT_GCB_PFN_vkCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders);
static void __HINT_GCB_PFN_vkDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_GCB_PFN_vkGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t *pDataSize, void *pData);
static void __HINT_GCB_PFN_vkCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount, const VkShaderStageFlagBits *pStages, const VkShaderEXT *pShaders);
static void __HINT_GCB_PFN_vkCmdSetDepthClampRangeEXT(VkCommandBuffer commandBuffer, VkDepthClampModeEXT depthClampMode, const VkDepthClampRangeEXT *pDepthClampRange);
static VkResult __HINT_GCB_PFN_vkGetFramebufferTilePropertiesQCOM(VkDevice device, VkFramebuffer framebuffer, uint32_t *pPropertiesCount, VkTilePropertiesQCOM *pProperties);
static VkResult __HINT_GCB_PFN_vkGetDynamicRenderingTilePropertiesQCOM(VkDevice device, const VkRenderingInfo *pRenderingInfo, VkTilePropertiesQCOM *pProperties);
static VkResult __HINT_GCB_PFN_vkGetPhysicalDeviceCooperativeVectorPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount, VkCooperativeVectorPropertiesNV *pProperties);
static VkResult __HINT_GCB_PFN_vkConvertCooperativeVectorMatrixNV(VkDevice device, const VkConvertCooperativeVectorMatrixInfoNV *pInfo);
static void __HINT_GCB_PFN_vkCmdConvertCooperativeVectorMatrixNV(VkCommandBuffer commandBuffer, uint32_t infoCount, const VkConvertCooperativeVectorMatrixInfoNV *pInfos);
static VkResult __HINT_GCB_PFN_vkSetLatencySleepModeNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepModeInfoNV *pSleepModeInfo);
static VkResult __HINT_GCB_PFN_vkLatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV *pSleepInfo);
static void __HINT_GCB_PFN_vkSetLatencyMarkerNV(VkDevice device, VkSwapchainKHR swapchain, const VkSetLatencyMarkerInfoNV *pLatencyMarkerInfo);
static void __HINT_GCB_PFN_vkGetLatencyTimingsNV(VkDevice device, VkSwapchainKHR swapchain, VkGetLatencyMarkerInfoNV *pLatencyMarkerInfo);
static void __HINT_GCB_PFN_vkQueueNotifyOutOfBandNV(VkQueue queue, const VkOutOfBandQueueTypeInfoNV *pQueueTypeInfo);
static void __HINT_GCB_PFN_vkCmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer commandBuffer, VkImageAspectFlags aspectMask);
static void __HINT_GCB_PFN_vkCmdBindTileMemoryQCOM(VkCommandBuffer commandBuffer, const VkTileMemoryBindInfoQCOM *pTileMemoryBindInfo);
static VkResult __HINT_GCB_PFN_vkCreateExternalComputeQueueNV(VkDevice device, const VkExternalComputeQueueCreateInfoNV *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkExternalComputeQueueNV *pExternalQueue);
static void __HINT_GCB_PFN_vkDestroyExternalComputeQueueNV(VkDevice device, VkExternalComputeQueueNV externalQueue, const VkAllocationCallbacks *pAllocator);
static void __HINT_GCB_PFN_vkGetExternalComputeQueueDataNV(VkExternalComputeQueueNV externalQueue, VkExternalComputeQueueDataParamsNV *params, void *pData);
static void __HINT_GCB_PFN_vkGetClusterAccelerationStructureBuildSizesNV(VkDevice device, const VkClusterAccelerationStructureInputInfoNV *pInfo, VkAccelerationStructureBuildSizesInfoKHR *pSizeInfo);
static void __HINT_GCB_PFN_vkCmdBuildClusterAccelerationStructureIndirectNV(VkCommandBuffer commandBuffer, const VkClusterAccelerationStructureCommandsInfoNV *pCommandInfos);
static void __HINT_GCB_PFN_vkGetPartitionedAccelerationStructuresBuildSizesNV(VkDevice device, const VkPartitionedAccelerationStructureInstancesInputNV *pInfo, VkAccelerationStructureBuildSizesInfoKHR *pSizeInfo);
static void __HINT_GCB_PFN_vkCmdBuildPartitionedAccelerationStructuresNV(VkCommandBuffer commandBuffer, const VkBuildPartitionedAccelerationStructureInfoNV *pBuildInfo);
static void __HINT_GCB_PFN_vkGetGeneratedCommandsMemoryRequirementsEXT(VkDevice device, const VkGeneratedCommandsMemoryRequirementsInfoEXT *pInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __HINT_GCB_PFN_vkCmdPreprocessGeneratedCommandsEXT(VkCommandBuffer commandBuffer, const VkGeneratedCommandsInfoEXT *pGeneratedCommandsInfo, VkCommandBuffer stateCommandBuffer);
static void __HINT_GCB_PFN_vkCmdExecuteGeneratedCommandsEXT(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed, const VkGeneratedCommandsInfoEXT *pGeneratedCommandsInfo);
static VkResult __HINT_GCB_PFN_vkCreateIndirectCommandsLayoutEXT(VkDevice device, const VkIndirectCommandsLayoutCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkIndirectCommandsLayoutEXT *pIndirectCommandsLayout);
static void __HINT_GCB_PFN_vkDestroyIndirectCommandsLayoutEXT(VkDevice device, VkIndirectCommandsLayoutEXT indirectCommandsLayout, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_GCB_PFN_vkCreateIndirectExecutionSetEXT(VkDevice device, const VkIndirectExecutionSetCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkIndirectExecutionSetEXT *pIndirectExecutionSet);
static void __HINT_GCB_PFN_vkDestroyIndirectExecutionSetEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet, const VkAllocationCallbacks *pAllocator);
static void __HINT_GCB_PFN_vkUpdateIndirectExecutionSetPipelineEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet, uint32_t executionSetWriteCount, const VkWriteIndirectExecutionSetPipelineEXT *pExecutionSetWrites);
static void __HINT_GCB_PFN_vkUpdateIndirectExecutionSetShaderEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet, uint32_t executionSetWriteCount, const VkWriteIndirectExecutionSetShaderEXT *pExecutionSetWrites);
static VkResult __HINT_GCB_PFN_vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount, VkCooperativeMatrixFlexibleDimensionsPropertiesNV *pProperties);
static void __HINT_GCB_PFN_vkCmdEndRendering2EXT(VkCommandBuffer commandBuffer, const VkRenderingEndInfoEXT *pRenderingEndInfo);
static VkResult __HINT_GCB_PFN_vkCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkAccelerationStructureKHR *pAccelerationStructure);
static void __HINT_GCB_PFN_vkDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure, const VkAllocationCallbacks *pAllocator);
static void __HINT_GCB_PFN_vkCmdBuildAccelerationStructuresKHR(VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos, const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos);
static void __HINT_GCB_PFN_vkCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos, const VkDeviceAddress *pIndirectDeviceAddresses, const uint32_t *pIndirectStrides, const uint32_t *const *ppMaxPrimitiveCounts);
static VkResult __HINT_GCB_PFN_vkBuildAccelerationStructuresKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos, const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos);
static VkResult __HINT_GCB_PFN_vkCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyAccelerationStructureInfoKHR *pInfo);
static VkResult __HINT_GCB_PFN_vkCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo);
static VkResult __HINT_GCB_PFN_vkCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo);
static VkResult __HINT_GCB_PFN_vkWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR *pAccelerationStructures, VkQueryType queryType, size_t dataSize, void *pData, size_t stride);
static void __HINT_GCB_PFN_vkCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureInfoKHR *pInfo);
static void __HINT_GCB_PFN_vkCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo);
static void __HINT_GCB_PFN_vkCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer, const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo);
static VkDeviceAddress __HINT_GCB_PFN_vkGetAccelerationStructureDeviceAddressKHR(VkDevice device, const VkAccelerationStructureDeviceAddressInfoKHR *pInfo);
static void __HINT_GCB_PFN_vkCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR *pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery);
static void __HINT_GCB_PFN_vkGetDeviceAccelerationStructureCompatibilityKHR(VkDevice device, const VkAccelerationStructureVersionInfoKHR *pVersionInfo, VkAccelerationStructureCompatibilityKHR *pCompatibility);
static void __HINT_GCB_PFN_vkGetAccelerationStructureBuildSizesKHR(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType, const VkAccelerationStructureBuildGeometryInfoKHR *pBuildInfo, const uint32_t *pMaxPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR *pSizeInfo);
static void __HINT_GCB_PFN_vkCmdTraceRaysKHR(VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable, const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable, const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable, const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable, uint32_t width, uint32_t height, uint32_t depth);
static VkResult __HINT_GCB_PFN_vkCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines);
static VkResult __HINT_GCB_PFN_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void *pData);
static void __HINT_GCB_PFN_vkCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable, const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable, const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable, const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable, VkDeviceAddress indirectDeviceAddress);
static VkDeviceSize __HINT_GCB_PFN_vkGetRayTracingShaderGroupStackSizeKHR(VkDevice device, VkPipeline pipeline, uint32_t group, VkShaderGroupShaderKHR groupShader);
static void __HINT_GCB_PFN_vkCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer, uint32_t pipelineStackSize);
static void __HINT_GCB_PFN_vkCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
static void __HINT_GCB_PFN_vkCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
static void __HINT_GCB_PFN_vkCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);
static VkResult __HINT_GCB_PFN_vkCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface);
static VkBool32 __HINT_GCB_PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, struct wl_display *display);
static VkResult __HINT_GCB_PFN_vkCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface);
static VkBool32 __HINT_GCB_PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, xcb_connection_t *connection, xcb_visualid_t visual_id);
static VkResult __HINT_GCB_PFN_vkCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface);
static VkBool32 __HINT_GCB_PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, Display *dpy, VisualID visualID);

// HCBs
static VkResult __HINT_HCB_PFN_vkCreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkInstance *pInstance);
static void __HINT_HCB_PFN_vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_HCB_PFN_vkEnumeratePhysicalDevices(VkInstance instance, uint32_t *pPhysicalDeviceCount, VkPhysicalDevice *pPhysicalDevices);
static void __HINT_HCB_PFN_vkGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures *pFeatures);
static void __HINT_HCB_PFN_vkGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties *pFormatProperties);
static VkResult __HINT_HCB_PFN_vkGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkImageFormatProperties *pImageFormatProperties);
static void __HINT_HCB_PFN_vkGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties *pProperties);
static void __HINT_HCB_PFN_vkGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t *pQueueFamilyPropertyCount, VkQueueFamilyProperties *pQueueFamilyProperties);
static void __HINT_HCB_PFN_vkGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties *pMemoryProperties);
static PFN_vkVoidFunction __HINT_HCB_PFN_vkGetInstanceProcAddr(VkInstance instance, const char *pName);
static PFN_vkVoidFunction __HINT_HCB_PFN_vkGetDeviceProcAddr(VkDevice device, const char *pName);
static VkResult __HINT_HCB_PFN_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDevice *pDevice);
static void __HINT_HCB_PFN_vkDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_HCB_PFN_vkEnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pPropertyCount, VkExtensionProperties *pProperties);
static VkResult __HINT_HCB_PFN_vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char *pLayerName, uint32_t *pPropertyCount, VkExtensionProperties *pProperties);
static VkResult __HINT_HCB_PFN_vkEnumerateInstanceLayerProperties(uint32_t *pPropertyCount, VkLayerProperties *pProperties);
static VkResult __HINT_HCB_PFN_vkEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount, VkLayerProperties *pProperties);
static void __HINT_HCB_PFN_vkGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue *pQueue);
static VkResult __HINT_HCB_PFN_vkQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence);
static VkResult __HINT_HCB_PFN_vkQueueWaitIdle(VkQueue queue);
static VkResult __HINT_HCB_PFN_vkDeviceWaitIdle(VkDevice device);
static VkResult __HINT_HCB_PFN_vkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo, const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory);
static void __HINT_HCB_PFN_vkFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_HCB_PFN_vkMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void **ppData);
static void __HINT_HCB_PFN_vkUnmapMemory(VkDevice device, VkDeviceMemory memory);
static VkResult __HINT_HCB_PFN_vkFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange *pMemoryRanges);
static VkResult __HINT_HCB_PFN_vkInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange *pMemoryRanges);
static void __HINT_HCB_PFN_vkGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize *pCommittedMemoryInBytes);
static VkResult __HINT_HCB_PFN_vkBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset);
static VkResult __HINT_HCB_PFN_vkBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset);
static void __HINT_HCB_PFN_vkGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements *pMemoryRequirements);
static void __HINT_HCB_PFN_vkGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements *pMemoryRequirements);
static void __HINT_HCB_PFN_vkGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t *pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements *pSparseMemoryRequirements);
static void __HINT_HCB_PFN_vkGetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t *pPropertyCount, VkSparseImageFormatProperties *pProperties);
static VkResult __HINT_HCB_PFN_vkQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo *pBindInfo, VkFence fence);
static VkResult __HINT_HCB_PFN_vkCreateFence(VkDevice device, const VkFenceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkFence *pFence);
static void __HINT_HCB_PFN_vkDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_HCB_PFN_vkResetFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences);
static VkResult __HINT_HCB_PFN_vkGetFenceStatus(VkDevice device, VkFence fence);
static VkResult __HINT_HCB_PFN_vkWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences, VkBool32 waitAll, uint64_t timeout);
static VkResult __HINT_HCB_PFN_vkCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSemaphore *pSemaphore);
static void __HINT_HCB_PFN_vkDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_HCB_PFN_vkCreateEvent(VkDevice device, const VkEventCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkEvent *pEvent);
static void __HINT_HCB_PFN_vkDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_HCB_PFN_vkGetEventStatus(VkDevice device, VkEvent event);
static VkResult __HINT_HCB_PFN_vkSetEvent(VkDevice device, VkEvent event);
static VkResult __HINT_HCB_PFN_vkResetEvent(VkDevice device, VkEvent event);
static VkResult __HINT_HCB_PFN_vkCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkQueryPool *pQueryPool);
static void __HINT_HCB_PFN_vkDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_HCB_PFN_vkGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void *pData, VkDeviceSize stride, VkQueryResultFlags flags);
static VkResult __HINT_HCB_PFN_vkCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer);
static void __HINT_HCB_PFN_vkDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_HCB_PFN_vkCreateBufferView(VkDevice device, const VkBufferViewCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkBufferView *pView);
static void __HINT_HCB_PFN_vkDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_HCB_PFN_vkCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkImage *pImage);
static void __HINT_HCB_PFN_vkDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator);
static void __HINT_HCB_PFN_vkGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource *pSubresource, VkSubresourceLayout *pLayout);
static VkResult __HINT_HCB_PFN_vkCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkImageView *pView);
static void __HINT_HCB_PFN_vkDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_HCB_PFN_vkCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkShaderModule *pShaderModule);
static void __HINT_HCB_PFN_vkDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_HCB_PFN_vkCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkPipelineCache *pPipelineCache);
static void __HINT_HCB_PFN_vkDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_HCB_PFN_vkGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t *pDataSize, void *pData);
static VkResult __HINT_HCB_PFN_vkMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount, const VkPipelineCache *pSrcCaches);
static VkResult __HINT_HCB_PFN_vkCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines);
static VkResult __HINT_HCB_PFN_vkCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines);
static void __HINT_HCB_PFN_vkDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_HCB_PFN_vkCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout);
static void __HINT_HCB_PFN_vkDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_HCB_PFN_vkCreateSampler(VkDevice device, const VkSamplerCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSampler *pSampler);
static void __HINT_HCB_PFN_vkDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_HCB_PFN_vkCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDescriptorSetLayout *pSetLayout);
static void __HINT_HCB_PFN_vkDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_HCB_PFN_vkCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDescriptorPool *pDescriptorPool);
static void __HINT_HCB_PFN_vkDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_HCB_PFN_vkResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags);
static VkResult __HINT_HCB_PFN_vkAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo *pAllocateInfo, VkDescriptorSet *pDescriptorSets);
static VkResult __HINT_HCB_PFN_vkFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet *pDescriptorSets);
static void __HINT_HCB_PFN_vkUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount, const VkWriteDescriptorSet *pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet *pDescriptorCopies);
static VkResult __HINT_HCB_PFN_vkCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkFramebuffer *pFramebuffer);
static void __HINT_HCB_PFN_vkDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_HCB_PFN_vkCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass);
static void __HINT_HCB_PFN_vkDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks *pAllocator);
static void __HINT_HCB_PFN_vkGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D *pGranularity);
static VkResult __HINT_HCB_PFN_vkCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkCommandPool *pCommandPool);
static void __HINT_HCB_PFN_vkDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_HCB_PFN_vkResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags);
static VkResult __HINT_HCB_PFN_vkAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo *pAllocateInfo, VkCommandBuffer *pCommandBuffers);
static void __HINT_HCB_PFN_vkFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer *pCommandBuffers);
static VkResult __HINT_HCB_PFN_vkBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *pBeginInfo);
static VkResult __HINT_HCB_PFN_vkEndCommandBuffer(VkCommandBuffer commandBuffer);
static VkResult __HINT_HCB_PFN_vkResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags);
static void __HINT_HCB_PFN_vkCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);
static void __HINT_HCB_PFN_vkCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport *pViewports);
static void __HINT_HCB_PFN_vkCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D *pScissors);
static void __HINT_HCB_PFN_vkCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth);
static void __HINT_HCB_PFN_vkCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor);
static void __HINT_HCB_PFN_vkCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]);
static void __HINT_HCB_PFN_vkCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds);
static void __HINT_HCB_PFN_vkCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask);
static void __HINT_HCB_PFN_vkCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask);
static void __HINT_HCB_PFN_vkCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference);
static void __HINT_HCB_PFN_vkCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet *pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t *pDynamicOffsets);
static void __HINT_HCB_PFN_vkCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType);
static void __HINT_HCB_PFN_vkCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer *pBuffers, const VkDeviceSize *pOffsets);
static void __HINT_HCB_PFN_vkCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
static void __HINT_HCB_PFN_vkCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
static void __HINT_HCB_PFN_vkCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
static void __HINT_HCB_PFN_vkCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
static void __HINT_HCB_PFN_vkCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
static void __HINT_HCB_PFN_vkCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset);
static void __HINT_HCB_PFN_vkCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy *pRegions);
static void __HINT_HCB_PFN_vkCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy *pRegions);
static void __HINT_HCB_PFN_vkCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit *pRegions, VkFilter filter);
static void __HINT_HCB_PFN_vkCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy *pRegions);
static void __HINT_HCB_PFN_vkCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions);
static void __HINT_HCB_PFN_vkCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void *pData);
static void __HINT_HCB_PFN_vkCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data);
static void __HINT_HCB_PFN_vkCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue *pColor, uint32_t rangeCount, const VkImageSubresourceRange *pRanges);
static void __HINT_HCB_PFN_vkCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange *pRanges);
static void __HINT_HCB_PFN_vkCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment *pAttachments, uint32_t rectCount, const VkClearRect *pRects);
static void __HINT_HCB_PFN_vkCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve *pRegions);
static void __HINT_HCB_PFN_vkCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask);
static void __HINT_HCB_PFN_vkCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask);
static void __HINT_HCB_PFN_vkCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers);
static void __HINT_HCB_PFN_vkCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers);
static void __HINT_HCB_PFN_vkCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags);
static void __HINT_HCB_PFN_vkCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query);
static void __HINT_HCB_PFN_vkCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);
static void __HINT_HCB_PFN_vkCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query);
static void __HINT_HCB_PFN_vkCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags);
static void __HINT_HCB_PFN_vkCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void *pValues);
static void __HINT_HCB_PFN_vkCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin, VkSubpassContents contents);
static void __HINT_HCB_PFN_vkCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents);
static void __HINT_HCB_PFN_vkCmdEndRenderPass(VkCommandBuffer commandBuffer);
static void __HINT_HCB_PFN_vkCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount, const VkCommandBuffer *pCommandBuffers);
static VkResult __HINT_HCB_PFN_vkEnumerateInstanceVersion(uint32_t *pApiVersion);
static VkResult __HINT_HCB_PFN_vkBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo *pBindInfos);
static VkResult __HINT_HCB_PFN_vkBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo *pBindInfos);
static void __HINT_HCB_PFN_vkGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags *pPeerMemoryFeatures);
static void __HINT_HCB_PFN_vkCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask);
static void __HINT_HCB_PFN_vkCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
static VkResult __HINT_HCB_PFN_vkEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t *pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroupProperties);
static void __HINT_HCB_PFN_vkGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2 *pInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __HINT_HCB_PFN_vkGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2 *pInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __HINT_HCB_PFN_vkGetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2 *pInfo, uint32_t *pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements);
static void __HINT_HCB_PFN_vkGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2 *pFeatures);
static void __HINT_HCB_PFN_vkGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2 *pProperties);
static void __HINT_HCB_PFN_vkGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2 *pFormatProperties);
static VkResult __HINT_HCB_PFN_vkGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo, VkImageFormatProperties2 *pImageFormatProperties);
static void __HINT_HCB_PFN_vkGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t *pQueueFamilyPropertyCount, VkQueueFamilyProperties2 *pQueueFamilyProperties);
static void __HINT_HCB_PFN_vkGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2 *pMemoryProperties);
static void __HINT_HCB_PFN_vkGetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2 *pFormatInfo, uint32_t *pPropertyCount, VkSparseImageFormatProperties2 *pProperties);
static void __HINT_HCB_PFN_vkTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags);
static void __HINT_HCB_PFN_vkGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2 *pQueueInfo, VkQueue *pQueue);
static VkResult __HINT_HCB_PFN_vkCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSamplerYcbcrConversion *pYcbcrConversion);
static void __HINT_HCB_PFN_vkDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_HCB_PFN_vkCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate);
static void __HINT_HCB_PFN_vkDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks *pAllocator);
static void __HINT_HCB_PFN_vkUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void *pData);
static void __HINT_HCB_PFN_vkGetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo *pExternalBufferInfo, VkExternalBufferProperties *pExternalBufferProperties);
static void __HINT_HCB_PFN_vkGetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo *pExternalFenceInfo, VkExternalFenceProperties *pExternalFenceProperties);
static void __HINT_HCB_PFN_vkGetPhysicalDeviceExternalSemaphoreProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo *pExternalSemaphoreInfo, VkExternalSemaphoreProperties *pExternalSemaphoreProperties);
static void __HINT_HCB_PFN_vkGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo, VkDescriptorSetLayoutSupport *pSupport);
static void __HINT_HCB_PFN_vkCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);
static void __HINT_HCB_PFN_vkCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);
static VkResult __HINT_HCB_PFN_vkCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass);
static void __HINT_HCB_PFN_vkCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin, const VkSubpassBeginInfo *pSubpassBeginInfo);
static void __HINT_HCB_PFN_vkCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo, const VkSubpassEndInfo *pSubpassEndInfo);
static void __HINT_HCB_PFN_vkCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo);
static void __HINT_HCB_PFN_vkResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);
static VkResult __HINT_HCB_PFN_vkGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t *pValue);
static VkResult __HINT_HCB_PFN_vkWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout);
static VkResult __HINT_HCB_PFN_vkSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo);
static VkDeviceAddress __HINT_HCB_PFN_vkGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo *pInfo);
static uint64_t __HINT_HCB_PFN_vkGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo *pInfo);
static uint64_t __HINT_HCB_PFN_vkGetDeviceMemoryOpaqueCaptureAddress(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo *pInfo);
static VkResult __HINT_HCB_PFN_vkGetPhysicalDeviceToolProperties(VkPhysicalDevice physicalDevice, uint32_t *pToolCount, VkPhysicalDeviceToolProperties *pToolProperties);
static VkResult __HINT_HCB_PFN_vkCreatePrivateDataSlot(VkDevice device, const VkPrivateDataSlotCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkPrivateDataSlot *pPrivateDataSlot);
static void __HINT_HCB_PFN_vkDestroyPrivateDataSlot(VkDevice device, VkPrivateDataSlot privateDataSlot, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_HCB_PFN_vkSetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlot privateDataSlot, uint64_t data);
static void __HINT_HCB_PFN_vkGetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlot privateDataSlot, uint64_t *pData);
static void __HINT_HCB_PFN_vkCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo *pDependencyInfo);
static void __HINT_HCB_PFN_vkCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask);
static void __HINT_HCB_PFN_vkCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents, const VkDependencyInfo *pDependencyInfos);
static void __HINT_HCB_PFN_vkCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo);
static void __HINT_HCB_PFN_vkCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool, uint32_t query);
static VkResult __HINT_HCB_PFN_vkQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits, VkFence fence);
static void __HINT_HCB_PFN_vkCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo);
static void __HINT_HCB_PFN_vkCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo);
static void __HINT_HCB_PFN_vkCmdCopyBufferToImage2(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo);
static void __HINT_HCB_PFN_vkCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo);
static void __HINT_HCB_PFN_vkCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2 *pBlitImageInfo);
static void __HINT_HCB_PFN_vkCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2 *pResolveImageInfo);
static void __HINT_HCB_PFN_vkCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo);
static void __HINT_HCB_PFN_vkCmdEndRendering(VkCommandBuffer commandBuffer);
static void __HINT_HCB_PFN_vkCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode);
static void __HINT_HCB_PFN_vkCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace);
static void __HINT_HCB_PFN_vkCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology);
static void __HINT_HCB_PFN_vkCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport *pViewports);
static void __HINT_HCB_PFN_vkCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D *pScissors);
static void __HINT_HCB_PFN_vkCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer *pBuffers, const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes, const VkDeviceSize *pStrides);
static void __HINT_HCB_PFN_vkCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable);
static void __HINT_HCB_PFN_vkCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable);
static void __HINT_HCB_PFN_vkCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp);
static void __HINT_HCB_PFN_vkCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable);
static void __HINT_HCB_PFN_vkCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable);
static void __HINT_HCB_PFN_vkCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp);
static void __HINT_HCB_PFN_vkCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable);
static void __HINT_HCB_PFN_vkCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable);
static void __HINT_HCB_PFN_vkCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable);
static void __HINT_HCB_PFN_vkGetDeviceBufferMemoryRequirements(VkDevice device, const VkDeviceBufferMemoryRequirements *pInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __HINT_HCB_PFN_vkGetDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements *pInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __HINT_HCB_PFN_vkGetDeviceImageSparseMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements *pInfo, uint32_t *pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements);
static void __HINT_HCB_PFN_vkCmdSetLineStipple(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern);
static VkResult __HINT_HCB_PFN_vkMapMemory2(VkDevice device, const VkMemoryMapInfo *pMemoryMapInfo, void **ppData);
static VkResult __HINT_HCB_PFN_vkUnmapMemory2(VkDevice device, const VkMemoryUnmapInfo *pMemoryUnmapInfo);
static void __HINT_HCB_PFN_vkCmdBindIndexBuffer2(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size, VkIndexType indexType);
static void __HINT_HCB_PFN_vkGetRenderingAreaGranularity(VkDevice device, const VkRenderingAreaInfo *pRenderingAreaInfo, VkExtent2D *pGranularity);
static void __HINT_HCB_PFN_vkGetDeviceImageSubresourceLayout(VkDevice device, const VkDeviceImageSubresourceInfo *pInfo, VkSubresourceLayout2 *pLayout);
static void __HINT_HCB_PFN_vkGetImageSubresourceLayout2(VkDevice device, VkImage image, const VkImageSubresource2 *pSubresource, VkSubresourceLayout2 *pLayout);
static void __HINT_HCB_PFN_vkCmdPushDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount, const VkWriteDescriptorSet *pDescriptorWrites);
static void __HINT_HCB_PFN_vkCmdPushDescriptorSetWithTemplate(VkCommandBuffer commandBuffer, VkDescriptorUpdateTemplate descriptorUpdateTemplate, VkPipelineLayout layout, uint32_t set, const void *pData);
static void __HINT_HCB_PFN_vkCmdSetRenderingAttachmentLocations(VkCommandBuffer commandBuffer, const VkRenderingAttachmentLocationInfo *pLocationInfo);
static void __HINT_HCB_PFN_vkCmdSetRenderingInputAttachmentIndices(VkCommandBuffer commandBuffer, const VkRenderingInputAttachmentIndexInfo *pInputAttachmentIndexInfo);
static void __HINT_HCB_PFN_vkCmdBindDescriptorSets2(VkCommandBuffer commandBuffer, const VkBindDescriptorSetsInfo *pBindDescriptorSetsInfo);
static void __HINT_HCB_PFN_vkCmdPushConstants2(VkCommandBuffer commandBuffer, const VkPushConstantsInfo *pPushConstantsInfo);
static void __HINT_HCB_PFN_vkCmdPushDescriptorSet2(VkCommandBuffer commandBuffer, const VkPushDescriptorSetInfo *pPushDescriptorSetInfo);
static void __HINT_HCB_PFN_vkCmdPushDescriptorSetWithTemplate2(VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfo *pPushDescriptorSetWithTemplateInfo);
static VkResult __HINT_HCB_PFN_vkCopyMemoryToImage(VkDevice device, const VkCopyMemoryToImageInfo *pCopyMemoryToImageInfo);
static VkResult __HINT_HCB_PFN_vkCopyImageToMemory(VkDevice device, const VkCopyImageToMemoryInfo *pCopyImageToMemoryInfo);
static VkResult __HINT_HCB_PFN_vkCopyImageToImage(VkDevice device, const VkCopyImageToImageInfo *pCopyImageToImageInfo);
static VkResult __HINT_HCB_PFN_vkTransitionImageLayout(VkDevice device, uint32_t transitionCount, const VkHostImageLayoutTransitionInfo *pTransitions);
static void __HINT_HCB_PFN_vkDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_HCB_PFN_vkGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32 *pSupported);
static VkResult __HINT_HCB_PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR *pSurfaceCapabilities);
static VkResult __HINT_HCB_PFN_vkGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t *pSurfaceFormatCount, VkSurfaceFormatKHR *pSurfaceFormats);
static VkResult __HINT_HCB_PFN_vkGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t *pPresentModeCount, VkPresentModeKHR *pPresentModes);
static VkResult __HINT_HCB_PFN_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchain);
static void __HINT_HCB_PFN_vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_HCB_PFN_vkGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t *pSwapchainImageCount, VkImage *pSwapchainImages);
static VkResult __HINT_HCB_PFN_vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex);
static VkResult __HINT_HCB_PFN_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo);
static VkResult __HINT_HCB_PFN_vkGetDeviceGroupPresentCapabilitiesKHR(VkDevice device, VkDeviceGroupPresentCapabilitiesKHR *pDeviceGroupPresentCapabilities);
static VkResult __HINT_HCB_PFN_vkGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface, VkDeviceGroupPresentModeFlagsKHR *pModes);
static VkResult __HINT_HCB_PFN_vkGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t *pRectCount, VkRect2D *pRects);
static VkResult __HINT_HCB_PFN_vkAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR *pAcquireInfo, uint32_t *pImageIndex);
static VkResult __HINT_HCB_PFN_vkGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount, VkDisplayPropertiesKHR *pProperties);
static VkResult __HINT_HCB_PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount, VkDisplayPlanePropertiesKHR *pProperties);
static VkResult __HINT_HCB_PFN_vkGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex, uint32_t *pDisplayCount, VkDisplayKHR *pDisplays);
static VkResult __HINT_HCB_PFN_vkGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t *pPropertyCount, VkDisplayModePropertiesKHR *pProperties);
static VkResult __HINT_HCB_PFN_vkCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, const VkDisplayModeCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDisplayModeKHR *pMode);
static VkResult __HINT_HCB_PFN_vkGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR *pCapabilities);
static VkResult __HINT_HCB_PFN_vkCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface);
static VkResult __HINT_HCB_PFN_vkCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount, const VkSwapchainCreateInfoKHR *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchains);
static VkResult __HINT_HCB_PFN_vkGetPhysicalDeviceVideoCapabilitiesKHR(VkPhysicalDevice physicalDevice, const VkVideoProfileInfoKHR *pVideoProfile, VkVideoCapabilitiesKHR *pCapabilities);
static VkResult __HINT_HCB_PFN_vkGetPhysicalDeviceVideoFormatPropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoFormatInfoKHR *pVideoFormatInfo, uint32_t *pVideoFormatPropertyCount, VkVideoFormatPropertiesKHR *pVideoFormatProperties);
static VkResult __HINT_HCB_PFN_vkCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkVideoSessionKHR *pVideoSession);
static void __HINT_HCB_PFN_vkDestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_HCB_PFN_vkGetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession, uint32_t *pMemoryRequirementsCount, VkVideoSessionMemoryRequirementsKHR *pMemoryRequirements);
static VkResult __HINT_HCB_PFN_vkBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession, uint32_t bindSessionMemoryInfoCount, const VkBindVideoSessionMemoryInfoKHR *pBindSessionMemoryInfos);
static VkResult __HINT_HCB_PFN_vkCreateVideoSessionParametersKHR(VkDevice device, const VkVideoSessionParametersCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkVideoSessionParametersKHR *pVideoSessionParameters);
static VkResult __HINT_HCB_PFN_vkUpdateVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters, const VkVideoSessionParametersUpdateInfoKHR *pUpdateInfo);
static void __HINT_HCB_PFN_vkDestroyVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters, const VkAllocationCallbacks *pAllocator);
static void __HINT_HCB_PFN_vkCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR *pBeginInfo);
static void __HINT_HCB_PFN_vkCmdEndVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoEndCodingInfoKHR *pEndCodingInfo);
static void __HINT_HCB_PFN_vkCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoCodingControlInfoKHR *pCodingControlInfo);
static void __HINT_HCB_PFN_vkCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR *pDecodeInfo);
static void __HINT_HCB_PFN_vkCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo);
static void __HINT_HCB_PFN_vkCmdEndRenderingKHR(VkCommandBuffer commandBuffer);
static void __HINT_HCB_PFN_vkGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2 *pFeatures);
static void __HINT_HCB_PFN_vkGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2 *pProperties);
static void __HINT_HCB_PFN_vkGetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2 *pFormatProperties);
static VkResult __HINT_HCB_PFN_vkGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo, VkImageFormatProperties2 *pImageFormatProperties);
static void __HINT_HCB_PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t *pQueueFamilyPropertyCount, VkQueueFamilyProperties2 *pQueueFamilyProperties);
static void __HINT_HCB_PFN_vkGetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2 *pMemoryProperties);
static void __HINT_HCB_PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2 *pFormatInfo, uint32_t *pPropertyCount, VkSparseImageFormatProperties2 *pProperties);
static void __HINT_HCB_PFN_vkGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags *pPeerMemoryFeatures);
static void __HINT_HCB_PFN_vkCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask);
static void __HINT_HCB_PFN_vkCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
static void __HINT_HCB_PFN_vkTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags);
static VkResult __HINT_HCB_PFN_vkEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t *pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroupProperties);
static void __HINT_HCB_PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo *pExternalBufferInfo, VkExternalBufferProperties *pExternalBufferProperties);
static VkResult __HINT_HCB_PFN_vkGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR *pGetFdInfo, int *pFd);
static VkResult __HINT_HCB_PFN_vkGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd, VkMemoryFdPropertiesKHR *pMemoryFdProperties);
static void __HINT_HCB_PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo *pExternalSemaphoreInfo, VkExternalSemaphoreProperties *pExternalSemaphoreProperties);
static VkResult __HINT_HCB_PFN_vkImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR *pImportSemaphoreFdInfo);
static VkResult __HINT_HCB_PFN_vkGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR *pGetFdInfo, int *pFd);
static void __HINT_HCB_PFN_vkCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount, const VkWriteDescriptorSet *pDescriptorWrites);
static void __HINT_HCB_PFN_vkCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer, VkDescriptorUpdateTemplate descriptorUpdateTemplate, VkPipelineLayout layout, uint32_t set, const void *pData);
static VkResult __HINT_HCB_PFN_vkCreateDescriptorUpdateTemplateKHR(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate);
static void __HINT_HCB_PFN_vkDestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks *pAllocator);
static void __HINT_HCB_PFN_vkUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void *pData);
static VkResult __HINT_HCB_PFN_vkCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass);
static void __HINT_HCB_PFN_vkCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin, const VkSubpassBeginInfo *pSubpassBeginInfo);
static void __HINT_HCB_PFN_vkCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo, const VkSubpassEndInfo *pSubpassEndInfo);
static void __HINT_HCB_PFN_vkCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo);
static VkResult __HINT_HCB_PFN_vkGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain);
static void __HINT_HCB_PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo *pExternalFenceInfo, VkExternalFenceProperties *pExternalFenceProperties);
static VkResult __HINT_HCB_PFN_vkImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR *pImportFenceFdInfo);
static VkResult __HINT_HCB_PFN_vkGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR *pGetFdInfo, int *pFd);
static VkResult __HINT_HCB_PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t *pCounterCount, VkPerformanceCounterKHR *pCounters, VkPerformanceCounterDescriptionKHR *pCounterDescriptions);
static void __HINT_HCB_PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(VkPhysicalDevice physicalDevice, const VkQueryPoolPerformanceCreateInfoKHR *pPerformanceQueryCreateInfo, uint32_t *pNumPasses);
static VkResult __HINT_HCB_PFN_vkAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR *pInfo);
static void __HINT_HCB_PFN_vkReleaseProfilingLockKHR(VkDevice device);
static VkResult __HINT_HCB_PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo, VkSurfaceCapabilities2KHR *pSurfaceCapabilities);
static VkResult __HINT_HCB_PFN_vkGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo, uint32_t *pSurfaceFormatCount, VkSurfaceFormat2KHR *pSurfaceFormats);
static VkResult __HINT_HCB_PFN_vkGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount, VkDisplayProperties2KHR *pProperties);
static VkResult __HINT_HCB_PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount, VkDisplayPlaneProperties2KHR *pProperties);
static VkResult __HINT_HCB_PFN_vkGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t *pPropertyCount, VkDisplayModeProperties2KHR *pProperties);
static VkResult __HINT_HCB_PFN_vkGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice, const VkDisplayPlaneInfo2KHR *pDisplayPlaneInfo, VkDisplayPlaneCapabilities2KHR *pCapabilities);
static void __HINT_HCB_PFN_vkGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2 *pInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __HINT_HCB_PFN_vkGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2 *pInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __HINT_HCB_PFN_vkGetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2 *pInfo, uint32_t *pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements);
static VkResult __HINT_HCB_PFN_vkCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSamplerYcbcrConversion *pYcbcrConversion);
static void __HINT_HCB_PFN_vkDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_HCB_PFN_vkBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo *pBindInfos);
static VkResult __HINT_HCB_PFN_vkBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo *pBindInfos);
static void __HINT_HCB_PFN_vkGetDescriptorSetLayoutSupportKHR(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo, VkDescriptorSetLayoutSupport *pSupport);
static void __HINT_HCB_PFN_vkCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);
static void __HINT_HCB_PFN_vkCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);
static VkResult __HINT_HCB_PFN_vkGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t *pValue);
static VkResult __HINT_HCB_PFN_vkWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout);
static VkResult __HINT_HCB_PFN_vkSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo);
static VkResult __HINT_HCB_PFN_vkGetPhysicalDeviceFragmentShadingRatesKHR(VkPhysicalDevice physicalDevice, uint32_t *pFragmentShadingRateCount, VkPhysicalDeviceFragmentShadingRateKHR *pFragmentShadingRates);
static void __HINT_HCB_PFN_vkCmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer, const VkExtent2D *pFragmentSize, const VkFragmentShadingRateCombinerOpKHR combinerOps[2]);
static void __HINT_HCB_PFN_vkCmdSetRenderingAttachmentLocationsKHR(VkCommandBuffer commandBuffer, const VkRenderingAttachmentLocationInfo *pLocationInfo);
static void __HINT_HCB_PFN_vkCmdSetRenderingInputAttachmentIndicesKHR(VkCommandBuffer commandBuffer, const VkRenderingInputAttachmentIndexInfo *pInputAttachmentIndexInfo);
static VkResult __HINT_HCB_PFN_vkWaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId, uint64_t timeout);
static VkDeviceAddress __HINT_HCB_PFN_vkGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo *pInfo);
static uint64_t __HINT_HCB_PFN_vkGetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo *pInfo);
static uint64_t __HINT_HCB_PFN_vkGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo *pInfo);
static VkResult __HINT_HCB_PFN_vkCreateDeferredOperationKHR(VkDevice device, const VkAllocationCallbacks *pAllocator, VkDeferredOperationKHR *pDeferredOperation);
static void __HINT_HCB_PFN_vkDestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation, const VkAllocationCallbacks *pAllocator);
static uint32_t __HINT_HCB_PFN_vkGetDeferredOperationMaxConcurrencyKHR(VkDevice device, VkDeferredOperationKHR operation);
static VkResult __HINT_HCB_PFN_vkGetDeferredOperationResultKHR(VkDevice device, VkDeferredOperationKHR operation);
static VkResult __HINT_HCB_PFN_vkDeferredOperationJoinKHR(VkDevice device, VkDeferredOperationKHR operation);
static VkResult __HINT_HCB_PFN_vkGetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR *pPipelineInfo, uint32_t *pExecutableCount, VkPipelineExecutablePropertiesKHR *pProperties);
static VkResult __HINT_HCB_PFN_vkGetPipelineExecutableStatisticsKHR(VkDevice device, const VkPipelineExecutableInfoKHR *pExecutableInfo, uint32_t *pStatisticCount, VkPipelineExecutableStatisticKHR *pStatistics);
static VkResult __HINT_HCB_PFN_vkGetPipelineExecutableInternalRepresentationsKHR(VkDevice device, const VkPipelineExecutableInfoKHR *pExecutableInfo, uint32_t *pInternalRepresentationCount, VkPipelineExecutableInternalRepresentationKHR *pInternalRepresentations);
static VkResult __HINT_HCB_PFN_vkMapMemory2KHR(VkDevice device, const VkMemoryMapInfo *pMemoryMapInfo, void **ppData);
static VkResult __HINT_HCB_PFN_vkUnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfo *pMemoryUnmapInfo);
static VkResult __HINT_HCB_PFN_vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR *pQualityLevelInfo, VkVideoEncodeQualityLevelPropertiesKHR *pQualityLevelProperties);
static VkResult __HINT_HCB_PFN_vkGetEncodedVideoSessionParametersKHR(VkDevice device, const VkVideoEncodeSessionParametersGetInfoKHR *pVideoSessionParametersInfo, VkVideoEncodeSessionParametersFeedbackInfoKHR *pFeedbackInfo, size_t *pDataSize, void *pData);
static void __HINT_HCB_PFN_vkCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR *pEncodeInfo);
static void __HINT_HCB_PFN_vkCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo *pDependencyInfo);
static void __HINT_HCB_PFN_vkCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask);
static void __HINT_HCB_PFN_vkCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents, const VkDependencyInfo *pDependencyInfos);
static void __HINT_HCB_PFN_vkCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo);
static void __HINT_HCB_PFN_vkCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool, uint32_t query);
static VkResult __HINT_HCB_PFN_vkQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits, VkFence fence);
static void __HINT_HCB_PFN_vkCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo);
static void __HINT_HCB_PFN_vkCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo);
static void __HINT_HCB_PFN_vkCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo);
static void __HINT_HCB_PFN_vkCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo);
static void __HINT_HCB_PFN_vkCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2 *pBlitImageInfo);
static void __HINT_HCB_PFN_vkCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2 *pResolveImageInfo);
static void __HINT_HCB_PFN_vkCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress);
static void __HINT_HCB_PFN_vkGetDeviceBufferMemoryRequirementsKHR(VkDevice device, const VkDeviceBufferMemoryRequirements *pInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __HINT_HCB_PFN_vkGetDeviceImageMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements *pInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __HINT_HCB_PFN_vkGetDeviceImageSparseMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements *pInfo, uint32_t *pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements);
static void __HINT_HCB_PFN_vkCmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size, VkIndexType indexType);
static void __HINT_HCB_PFN_vkGetRenderingAreaGranularityKHR(VkDevice device, const VkRenderingAreaInfo *pRenderingAreaInfo, VkExtent2D *pGranularity);
static void __HINT_HCB_PFN_vkGetDeviceImageSubresourceLayoutKHR(VkDevice device, const VkDeviceImageSubresourceInfo *pInfo, VkSubresourceLayout2 *pLayout);
static void __HINT_HCB_PFN_vkGetImageSubresourceLayout2KHR(VkDevice device, VkImage image, const VkImageSubresource2 *pSubresource, VkSubresourceLayout2 *pLayout);
static VkResult __HINT_HCB_PFN_vkWaitForPresent2KHR(VkDevice device, VkSwapchainKHR swapchain, const VkPresentWait2InfoKHR *pPresentWait2Info);
static VkResult __HINT_HCB_PFN_vkCreatePipelineBinariesKHR(VkDevice device, const VkPipelineBinaryCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkPipelineBinaryHandlesInfoKHR *pBinaries);
static void __HINT_HCB_PFN_vkDestroyPipelineBinaryKHR(VkDevice device, VkPipelineBinaryKHR pipelineBinary, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_HCB_PFN_vkGetPipelineKeyKHR(VkDevice device, const VkPipelineCreateInfoKHR *pPipelineCreateInfo, VkPipelineBinaryKeyKHR *pPipelineKey);
static VkResult __HINT_HCB_PFN_vkGetPipelineBinaryDataKHR(VkDevice device, const VkPipelineBinaryDataInfoKHR *pInfo, VkPipelineBinaryKeyKHR *pPipelineBinaryKey, size_t *pPipelineBinaryDataSize, void *pPipelineBinaryData);
static VkResult __HINT_HCB_PFN_vkReleaseCapturedPipelineDataKHR(VkDevice device, const VkReleaseCapturedPipelineDataInfoKHR *pInfo, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_HCB_PFN_vkReleaseSwapchainImagesKHR(VkDevice device, const VkReleaseSwapchainImagesInfoKHR *pReleaseInfo);
static VkResult __HINT_HCB_PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount, VkCooperativeMatrixPropertiesKHR *pProperties);
static void __HINT_HCB_PFN_vkCmdSetLineStippleKHR(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern);
static VkResult __HINT_HCB_PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsKHR(VkPhysicalDevice physicalDevice, uint32_t *pTimeDomainCount, VkTimeDomainKHR *pTimeDomains);
static VkResult __HINT_HCB_PFN_vkGetCalibratedTimestampsKHR(VkDevice device, uint32_t timestampCount, const VkCalibratedTimestampInfoKHR *pTimestampInfos, uint64_t *pTimestamps, uint64_t *pMaxDeviation);
static void __HINT_HCB_PFN_vkCmdBindDescriptorSets2KHR(VkCommandBuffer commandBuffer, const VkBindDescriptorSetsInfo *pBindDescriptorSetsInfo);
static void __HINT_HCB_PFN_vkCmdPushConstants2KHR(VkCommandBuffer commandBuffer, const VkPushConstantsInfo *pPushConstantsInfo);
static void __HINT_HCB_PFN_vkCmdPushDescriptorSet2KHR(VkCommandBuffer commandBuffer, const VkPushDescriptorSetInfo *pPushDescriptorSetInfo);
static void __HINT_HCB_PFN_vkCmdPushDescriptorSetWithTemplate2KHR(VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfo *pPushDescriptorSetWithTemplateInfo);
static void __HINT_HCB_PFN_vkCmdSetDescriptorBufferOffsets2EXT(VkCommandBuffer commandBuffer, const VkSetDescriptorBufferOffsetsInfoEXT *pSetDescriptorBufferOffsetsInfo);
static void __HINT_HCB_PFN_vkCmdBindDescriptorBufferEmbeddedSamplers2EXT(VkCommandBuffer commandBuffer, const VkBindDescriptorBufferEmbeddedSamplersInfoEXT *pBindDescriptorBufferEmbeddedSamplersInfo);
static VkResult __HINT_HCB_PFN_vkCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugReportCallbackEXT *pCallback);
static void __HINT_HCB_PFN_vkDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks *pAllocator);
static void __HINT_HCB_PFN_vkDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char *pLayerPrefix, const char *pMessage);
static VkResult __HINT_HCB_PFN_vkDebugMarkerSetObjectTagEXT(VkDevice device, const VkDebugMarkerObjectTagInfoEXT *pTagInfo);
static VkResult __HINT_HCB_PFN_vkDebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT *pNameInfo);
static void __HINT_HCB_PFN_vkCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT *pMarkerInfo);
static void __HINT_HCB_PFN_vkCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer);
static void __HINT_HCB_PFN_vkCmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT *pMarkerInfo);
static void __HINT_HCB_PFN_vkCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer *pBuffers, const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes);
static void __HINT_HCB_PFN_vkCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer, uint32_t counterBufferCount, const VkBuffer *pCounterBuffers, const VkDeviceSize *pCounterBufferOffsets);
static void __HINT_HCB_PFN_vkCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer, uint32_t counterBufferCount, const VkBuffer *pCounterBuffers, const VkDeviceSize *pCounterBufferOffsets);
static void __HINT_HCB_PFN_vkCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags, uint32_t index);
static void __HINT_HCB_PFN_vkCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, uint32_t index);
static void __HINT_HCB_PFN_vkCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance, VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset, uint32_t vertexStride);
static VkResult __HINT_HCB_PFN_vkCreateCuModuleNVX(VkDevice device, const VkCuModuleCreateInfoNVX *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkCuModuleNVX *pModule);
static VkResult __HINT_HCB_PFN_vkCreateCuFunctionNVX(VkDevice device, const VkCuFunctionCreateInfoNVX *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkCuFunctionNVX *pFunction);
static void __HINT_HCB_PFN_vkDestroyCuModuleNVX(VkDevice device, VkCuModuleNVX module, const VkAllocationCallbacks *pAllocator);
static void __HINT_HCB_PFN_vkDestroyCuFunctionNVX(VkDevice device, VkCuFunctionNVX function, const VkAllocationCallbacks *pAllocator);
static void __HINT_HCB_PFN_vkCmdCuLaunchKernelNVX(VkCommandBuffer commandBuffer, const VkCuLaunchInfoNVX *pLaunchInfo);
static uint32_t __HINT_HCB_PFN_vkGetImageViewHandleNVX(VkDevice device, const VkImageViewHandleInfoNVX *pInfo);
static uint64_t __HINT_HCB_PFN_vkGetImageViewHandle64NVX(VkDevice device, const VkImageViewHandleInfoNVX *pInfo);
static VkResult __HINT_HCB_PFN_vkGetImageViewAddressNVX(VkDevice device, VkImageView imageView, VkImageViewAddressPropertiesNVX *pProperties);
static void __HINT_HCB_PFN_vkCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);
static void __HINT_HCB_PFN_vkCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);
static VkResult __HINT_HCB_PFN_vkGetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage, VkShaderInfoTypeAMD infoType, size_t *pInfoSize, void *pInfo);
static VkResult __HINT_HCB_PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType, VkExternalImageFormatPropertiesNV *pExternalImageFormatProperties);
static void __HINT_HCB_PFN_vkCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfoEXT *pConditionalRenderingBegin);
static void __HINT_HCB_PFN_vkCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer);
static void __HINT_HCB_PFN_vkCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewportWScalingNV *pViewportWScalings);
static VkResult __HINT_HCB_PFN_vkReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display);
static VkResult __HINT_HCB_PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilities2EXT *pSurfaceCapabilities);
static VkResult __HINT_HCB_PFN_vkDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display, const VkDisplayPowerInfoEXT *pDisplayPowerInfo);
static VkResult __HINT_HCB_PFN_vkRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT *pDeviceEventInfo, const VkAllocationCallbacks *pAllocator, VkFence *pFence);
static VkResult __HINT_HCB_PFN_vkRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display, const VkDisplayEventInfoEXT *pDisplayEventInfo, const VkAllocationCallbacks *pAllocator, VkFence *pFence);
static VkResult __HINT_HCB_PFN_vkGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter, uint64_t *pCounterValue);
static VkResult __HINT_HCB_PFN_vkGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain, VkRefreshCycleDurationGOOGLE *pDisplayTimingProperties);
static VkResult __HINT_HCB_PFN_vkGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain, uint32_t *pPresentationTimingCount, VkPastPresentationTimingGOOGLE *pPresentationTimings);
static void __HINT_HCB_PFN_vkCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle, uint32_t discardRectangleCount, const VkRect2D *pDiscardRectangles);
static void __HINT_HCB_PFN_vkCmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 discardRectangleEnable);
static void __HINT_HCB_PFN_vkCmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer, VkDiscardRectangleModeEXT discardRectangleMode);
static void __HINT_HCB_PFN_vkSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR *pSwapchains, const VkHdrMetadataEXT *pMetadata);
static VkResult __HINT_HCB_PFN_vkSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT *pNameInfo);
static VkResult __HINT_HCB_PFN_vkSetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT *pTagInfo);
static void __HINT_HCB_PFN_vkQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT *pLabelInfo);
static void __HINT_HCB_PFN_vkQueueEndDebugUtilsLabelEXT(VkQueue queue);
static void __HINT_HCB_PFN_vkQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT *pLabelInfo);
static void __HINT_HCB_PFN_vkCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT *pLabelInfo);
static void __HINT_HCB_PFN_vkCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer);
static void __HINT_HCB_PFN_vkCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT *pLabelInfo);
static VkResult __HINT_HCB_PFN_vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pMessenger);
static void __HINT_HCB_PFN_vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks *pAllocator);
static void __HINT_HCB_PFN_vkSubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData);
static void __HINT_HCB_PFN_vkCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer, const VkSampleLocationsInfoEXT *pSampleLocationsInfo);
static void __HINT_HCB_PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice physicalDevice, VkSampleCountFlagBits samples, VkMultisamplePropertiesEXT *pMultisampleProperties);
static VkResult __HINT_HCB_PFN_vkGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image, VkImageDrmFormatModifierPropertiesEXT *pProperties);
static VkResult __HINT_HCB_PFN_vkCreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkValidationCacheEXT *pValidationCache);
static void __HINT_HCB_PFN_vkDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_HCB_PFN_vkMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount, const VkValidationCacheEXT *pSrcCaches);
static VkResult __HINT_HCB_PFN_vkGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t *pDataSize, void *pData);
static void __HINT_HCB_PFN_vkCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout);
static void __HINT_HCB_PFN_vkCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkShadingRatePaletteNV *pShadingRatePalettes);
static void __HINT_HCB_PFN_vkCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType, uint32_t customSampleOrderCount, const VkCoarseSampleOrderCustomNV *pCustomSampleOrders);
static VkResult __HINT_HCB_PFN_vkCreateAccelerationStructureNV(VkDevice device, const VkAccelerationStructureCreateInfoNV *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkAccelerationStructureNV *pAccelerationStructure);
static void __HINT_HCB_PFN_vkDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure, const VkAllocationCallbacks *pAllocator);
static void __HINT_HCB_PFN_vkGetAccelerationStructureMemoryRequirementsNV(VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoNV *pInfo, VkMemoryRequirements2KHR *pMemoryRequirements);
static VkResult __HINT_HCB_PFN_vkBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount, const VkBindAccelerationStructureMemoryInfoNV *pBindInfos);
static void __HINT_HCB_PFN_vkCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV *pInfo, VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update, VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkBuffer scratch, VkDeviceSize scratchOffset);
static void __HINT_HCB_PFN_vkCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkCopyAccelerationStructureModeKHR mode);
static void __HINT_HCB_PFN_vkCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer, VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer, VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride, VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset, VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer, VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride, uint32_t width, uint32_t height, uint32_t depth);
static VkResult __HINT_HCB_PFN_vkCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoNV *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines);
static VkResult __HINT_HCB_PFN_vkGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void *pData);
static VkResult __HINT_HCB_PFN_vkGetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void *pData);
static VkResult __HINT_HCB_PFN_vkGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure, size_t dataSize, void *pData);
static void __HINT_HCB_PFN_vkCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureNV *pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery);
static VkResult __HINT_HCB_PFN_vkCompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader);
static VkResult __HINT_HCB_PFN_vkGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, const void *pHostPointer, VkMemoryHostPointerPropertiesEXT *pMemoryHostPointerProperties);
static void __HINT_HCB_PFN_vkCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker);
static void __HINT_HCB_PFN_vkCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker);
static VkResult __HINT_HCB_PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT(VkPhysicalDevice physicalDevice, uint32_t *pTimeDomainCount, VkTimeDomainKHR *pTimeDomains);
static VkResult __HINT_HCB_PFN_vkGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount, const VkCalibratedTimestampInfoKHR *pTimestampInfos, uint64_t *pTimestamps, uint64_t *pMaxDeviation);
static void __HINT_HCB_PFN_vkCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask);
static void __HINT_HCB_PFN_vkCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
static void __HINT_HCB_PFN_vkCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);
static void __HINT_HCB_PFN_vkCmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor, uint32_t exclusiveScissorCount, const VkBool32 *pExclusiveScissorEnables);
static void __HINT_HCB_PFN_vkCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor, uint32_t exclusiveScissorCount, const VkRect2D *pExclusiveScissors);
static void __HINT_HCB_PFN_vkCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void *pCheckpointMarker);
static void __HINT_HCB_PFN_vkGetQueueCheckpointDataNV(VkQueue queue, uint32_t *pCheckpointDataCount, VkCheckpointDataNV *pCheckpointData);
static void __HINT_HCB_PFN_vkGetQueueCheckpointData2NV(VkQueue queue, uint32_t *pCheckpointDataCount, VkCheckpointData2NV *pCheckpointData);
static VkResult __HINT_HCB_PFN_vkInitializePerformanceApiINTEL(VkDevice device, const VkInitializePerformanceApiInfoINTEL *pInitializeInfo);
static void __HINT_HCB_PFN_vkUninitializePerformanceApiINTEL(VkDevice device);
static VkResult __HINT_HCB_PFN_vkCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer, const VkPerformanceMarkerInfoINTEL *pMarkerInfo);
static VkResult __HINT_HCB_PFN_vkCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer, const VkPerformanceStreamMarkerInfoINTEL *pMarkerInfo);
static VkResult __HINT_HCB_PFN_vkCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer, const VkPerformanceOverrideInfoINTEL *pOverrideInfo);
static VkResult __HINT_HCB_PFN_vkAcquirePerformanceConfigurationINTEL(VkDevice device, const VkPerformanceConfigurationAcquireInfoINTEL *pAcquireInfo, VkPerformanceConfigurationINTEL *pConfiguration);
static VkResult __HINT_HCB_PFN_vkReleasePerformanceConfigurationINTEL(VkDevice device, VkPerformanceConfigurationINTEL configuration);
static VkResult __HINT_HCB_PFN_vkQueueSetPerformanceConfigurationINTEL(VkQueue queue, VkPerformanceConfigurationINTEL configuration);
static VkResult __HINT_HCB_PFN_vkGetPerformanceParameterINTEL(VkDevice device, VkPerformanceParameterTypeINTEL parameter, VkPerformanceValueINTEL *pValue);
static void __HINT_HCB_PFN_vkSetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapChain, VkBool32 localDimmingEnable);
static VkDeviceAddress __HINT_HCB_PFN_vkGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo *pInfo);
static VkResult __HINT_HCB_PFN_vkGetPhysicalDeviceToolPropertiesEXT(VkPhysicalDevice physicalDevice, uint32_t *pToolCount, VkPhysicalDeviceToolProperties *pToolProperties);
static VkResult __HINT_HCB_PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount, VkCooperativeMatrixPropertiesNV *pProperties);
static VkResult __HINT_HCB_PFN_vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(VkPhysicalDevice physicalDevice, uint32_t *pCombinationCount, VkFramebufferMixedSamplesCombinationNV *pCombinations);
static VkResult __HINT_HCB_PFN_vkCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface);
static void __HINT_HCB_PFN_vkCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern);
static void __HINT_HCB_PFN_vkResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);
static void __HINT_HCB_PFN_vkCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode);
static void __HINT_HCB_PFN_vkCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace);
static void __HINT_HCB_PFN_vkCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology);
static void __HINT_HCB_PFN_vkCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport *pViewports);
static void __HINT_HCB_PFN_vkCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D *pScissors);
static void __HINT_HCB_PFN_vkCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer *pBuffers, const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes, const VkDeviceSize *pStrides);
static void __HINT_HCB_PFN_vkCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable);
static void __HINT_HCB_PFN_vkCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable);
static void __HINT_HCB_PFN_vkCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp);
static void __HINT_HCB_PFN_vkCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable);
static void __HINT_HCB_PFN_vkCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable);
static void __HINT_HCB_PFN_vkCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp);
static VkResult __HINT_HCB_PFN_vkCopyMemoryToImageEXT(VkDevice device, const VkCopyMemoryToImageInfo *pCopyMemoryToImageInfo);
static VkResult __HINT_HCB_PFN_vkCopyImageToMemoryEXT(VkDevice device, const VkCopyImageToMemoryInfo *pCopyImageToMemoryInfo);
static VkResult __HINT_HCB_PFN_vkCopyImageToImageEXT(VkDevice device, const VkCopyImageToImageInfo *pCopyImageToImageInfo);
static VkResult __HINT_HCB_PFN_vkTransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount, const VkHostImageLayoutTransitionInfo *pTransitions);
static void __HINT_HCB_PFN_vkGetImageSubresourceLayout2EXT(VkDevice device, VkImage image, const VkImageSubresource2 *pSubresource, VkSubresourceLayout2 *pLayout);
static VkResult __HINT_HCB_PFN_vkReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoKHR *pReleaseInfo);
static void __HINT_HCB_PFN_vkGetGeneratedCommandsMemoryRequirementsNV(VkDevice device, const VkGeneratedCommandsMemoryRequirementsInfoNV *pInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __HINT_HCB_PFN_vkCmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer, const VkGeneratedCommandsInfoNV *pGeneratedCommandsInfo);
static void __HINT_HCB_PFN_vkCmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed, const VkGeneratedCommandsInfoNV *pGeneratedCommandsInfo);
static void __HINT_HCB_PFN_vkCmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline, uint32_t groupIndex);
static VkResult __HINT_HCB_PFN_vkCreateIndirectCommandsLayoutNV(VkDevice device, const VkIndirectCommandsLayoutCreateInfoNV *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkIndirectCommandsLayoutNV *pIndirectCommandsLayout);
static void __HINT_HCB_PFN_vkDestroyIndirectCommandsLayoutNV(VkDevice device, VkIndirectCommandsLayoutNV indirectCommandsLayout, const VkAllocationCallbacks *pAllocator);
static void __HINT_HCB_PFN_vkCmdSetDepthBias2EXT(VkCommandBuffer commandBuffer, const VkDepthBiasInfoEXT *pDepthBiasInfo);
static VkResult __HINT_HCB_PFN_vkAcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, VkDisplayKHR display);
static VkResult __HINT_HCB_PFN_vkGetDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, uint32_t connectorId, VkDisplayKHR *display);
static VkResult __HINT_HCB_PFN_vkCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkPrivateDataSlot *pPrivateDataSlot);
static void __HINT_HCB_PFN_vkDestroyPrivateDataSlotEXT(VkDevice device, VkPrivateDataSlot privateDataSlot, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_HCB_PFN_vkSetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlot privateDataSlot, uint64_t data);
static void __HINT_HCB_PFN_vkGetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlot privateDataSlot, uint64_t *pData);
static void __HINT_HCB_PFN_vkCmdDispatchTileQCOM(VkCommandBuffer commandBuffer, const VkDispatchTileInfoQCOM *pDispatchTileInfo);
static void __HINT_HCB_PFN_vkCmdBeginPerTileExecutionQCOM(VkCommandBuffer commandBuffer, const VkPerTileBeginInfoQCOM *pPerTileBeginInfo);
static void __HINT_HCB_PFN_vkCmdEndPerTileExecutionQCOM(VkCommandBuffer commandBuffer, const VkPerTileEndInfoQCOM *pPerTileEndInfo);
static void __HINT_HCB_PFN_vkGetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout, VkDeviceSize *pLayoutSizeInBytes);
static void __HINT_HCB_PFN_vkGetDescriptorSetLayoutBindingOffsetEXT(VkDevice device, VkDescriptorSetLayout layout, uint32_t binding, VkDeviceSize *pOffset);
static void __HINT_HCB_PFN_vkGetDescriptorEXT(VkDevice device, const VkDescriptorGetInfoEXT *pDescriptorInfo, size_t dataSize, void *pDescriptor);
static void __HINT_HCB_PFN_vkCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount, const VkDescriptorBufferBindingInfoEXT *pBindingInfos);
static void __HINT_HCB_PFN_vkCmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount, const uint32_t *pBufferIndices, const VkDeviceSize *pOffsets);
static void __HINT_HCB_PFN_vkCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t set);
static VkResult __HINT_HCB_PFN_vkGetBufferOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkBufferCaptureDescriptorDataInfoEXT *pInfo, void *pData);
static VkResult __HINT_HCB_PFN_vkGetImageOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkImageCaptureDescriptorDataInfoEXT *pInfo, void *pData);
static VkResult __HINT_HCB_PFN_vkGetImageViewOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkImageViewCaptureDescriptorDataInfoEXT *pInfo, void *pData);
static VkResult __HINT_HCB_PFN_vkGetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkSamplerCaptureDescriptorDataInfoEXT *pInfo, void *pData);
static VkResult __HINT_HCB_PFN_vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkAccelerationStructureCaptureDescriptorDataInfoEXT *pInfo, void *pData);
static void __HINT_HCB_PFN_vkCmdSetFragmentShadingRateEnumNV(VkCommandBuffer commandBuffer, VkFragmentShadingRateNV shadingRate, const VkFragmentShadingRateCombinerOpKHR combinerOps[2]);
static VkResult __HINT_HCB_PFN_vkGetDeviceFaultInfoEXT(VkDevice device, VkDeviceFaultCountsEXT *pFaultCounts, VkDeviceFaultInfoEXT *pFaultInfo);
static void __HINT_HCB_PFN_vkCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount, const VkVertexInputBindingDescription2EXT *pVertexBindingDescriptions, uint32_t vertexAttributeDescriptionCount, const VkVertexInputAttributeDescription2EXT *pVertexAttributeDescriptions);
static VkResult __HINT_HCB_PFN_vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(VkDevice device, VkRenderPass renderpass, VkExtent2D *pMaxWorkgroupSize);
static void __HINT_HCB_PFN_vkCmdSubpassShadingHUAWEI(VkCommandBuffer commandBuffer);
static void __HINT_HCB_PFN_vkCmdBindInvocationMaskHUAWEI(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout);
static VkResult __HINT_HCB_PFN_vkGetMemoryRemoteAddressNV(VkDevice device, const VkMemoryGetRemoteAddressInfoNV *pMemoryGetRemoteAddressInfo, VkRemoteAddressNV *pAddress);
static VkResult __HINT_HCB_PFN_vkGetPipelinePropertiesEXT(VkDevice device, const VkPipelineInfoEXT *pPipelineInfo, VkBaseOutStructure *pPipelineProperties);
static void __HINT_HCB_PFN_vkCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints);
static void __HINT_HCB_PFN_vkCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable);
static void __HINT_HCB_PFN_vkCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable);
static void __HINT_HCB_PFN_vkCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp);
static void __HINT_HCB_PFN_vkCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable);
static void __HINT_HCB_PFN_vkCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkBool32 *pColorWriteEnables);
static void __HINT_HCB_PFN_vkCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT *pVertexInfo, uint32_t instanceCount, uint32_t firstInstance, uint32_t stride);
static void __HINT_HCB_PFN_vkCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawIndexedInfoEXT *pIndexInfo, uint32_t instanceCount, uint32_t firstInstance, uint32_t stride, const int32_t *pVertexOffset);
static VkResult __HINT_HCB_PFN_vkCreateMicromapEXT(VkDevice device, const VkMicromapCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkMicromapEXT *pMicromap);
static void __HINT_HCB_PFN_vkDestroyMicromapEXT(VkDevice device, VkMicromapEXT micromap, const VkAllocationCallbacks *pAllocator);
static void __HINT_HCB_PFN_vkCmdBuildMicromapsEXT(VkCommandBuffer commandBuffer, uint32_t infoCount, const VkMicromapBuildInfoEXT *pInfos);
static VkResult __HINT_HCB_PFN_vkBuildMicromapsEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount, const VkMicromapBuildInfoEXT *pInfos);
static VkResult __HINT_HCB_PFN_vkCopyMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyMicromapInfoEXT *pInfo);
static VkResult __HINT_HCB_PFN_vkCopyMicromapToMemoryEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyMicromapToMemoryInfoEXT *pInfo);
static VkResult __HINT_HCB_PFN_vkCopyMemoryToMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyMemoryToMicromapInfoEXT *pInfo);
static VkResult __HINT_HCB_PFN_vkWriteMicromapsPropertiesEXT(VkDevice device, uint32_t micromapCount, const VkMicromapEXT *pMicromaps, VkQueryType queryType, size_t dataSize, void *pData, size_t stride);
static void __HINT_HCB_PFN_vkCmdCopyMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapInfoEXT *pInfo);
static void __HINT_HCB_PFN_vkCmdCopyMicromapToMemoryEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapToMemoryInfoEXT *pInfo);
static void __HINT_HCB_PFN_vkCmdCopyMemoryToMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMemoryToMicromapInfoEXT *pInfo);
static void __HINT_HCB_PFN_vkCmdWriteMicromapsPropertiesEXT(VkCommandBuffer commandBuffer, uint32_t micromapCount, const VkMicromapEXT *pMicromaps, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery);
static void __HINT_HCB_PFN_vkGetDeviceMicromapCompatibilityEXT(VkDevice device, const VkMicromapVersionInfoEXT *pVersionInfo, VkAccelerationStructureCompatibilityKHR *pCompatibility);
static void __HINT_HCB_PFN_vkGetMicromapBuildSizesEXT(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType, const VkMicromapBuildInfoEXT *pBuildInfo, VkMicromapBuildSizesInfoEXT *pSizeInfo);
static void __HINT_HCB_PFN_vkCmdDrawClusterHUAWEI(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
static void __HINT_HCB_PFN_vkCmdDrawClusterIndirectHUAWEI(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset);
static void __HINT_HCB_PFN_vkSetDeviceMemoryPriorityEXT(VkDevice device, VkDeviceMemory memory, float priority);
static void __HINT_HCB_PFN_vkGetDescriptorSetLayoutHostMappingInfoVALVE(VkDevice device, const VkDescriptorSetBindingReferenceVALVE *pBindingReference, VkDescriptorSetLayoutHostMappingInfoVALVE *pHostMapping);
static void __HINT_HCB_PFN_vkGetDescriptorSetHostMappingVALVE(VkDevice device, VkDescriptorSet descriptorSet, void **ppData);
static void __HINT_HCB_PFN_vkCmdCopyMemoryIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress, uint32_t copyCount, uint32_t stride);
static void __HINT_HCB_PFN_vkCmdCopyMemoryToImageIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress, uint32_t copyCount, uint32_t stride, VkImage dstImage, VkImageLayout dstImageLayout, const VkImageSubresourceLayers *pImageSubresources);
static void __HINT_HCB_PFN_vkCmdDecompressMemoryNV(VkCommandBuffer commandBuffer, uint32_t decompressRegionCount, const VkDecompressMemoryRegionNV *pDecompressMemoryRegions);
static void __HINT_HCB_PFN_vkCmdDecompressMemoryIndirectCountNV(VkCommandBuffer commandBuffer, VkDeviceAddress indirectCommandsAddress, VkDeviceAddress indirectCommandsCountAddress, uint32_t stride);
static void __HINT_HCB_PFN_vkGetPipelineIndirectMemoryRequirementsNV(VkDevice device, const VkComputePipelineCreateInfo *pCreateInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __HINT_HCB_PFN_vkCmdUpdatePipelineIndirectBufferNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);
static VkDeviceAddress __HINT_HCB_PFN_vkGetPipelineIndirectDeviceAddressNV(VkDevice device, const VkPipelineIndirectDeviceAddressInfoNV *pInfo);
static void __HINT_HCB_PFN_vkCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable);
static void __HINT_HCB_PFN_vkCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode);
static void __HINT_HCB_PFN_vkCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits rasterizationSamples);
static void __HINT_HCB_PFN_vkCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples, const VkSampleMask *pSampleMask);
static void __HINT_HCB_PFN_vkCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToCoverageEnable);
static void __HINT_HCB_PFN_vkCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable);
static void __HINT_HCB_PFN_vkCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable);
static void __HINT_HCB_PFN_vkCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount, const VkBool32 *pColorBlendEnables);
static void __HINT_HCB_PFN_vkCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount, const VkColorBlendEquationEXT *pColorBlendEquations);
static void __HINT_HCB_PFN_vkCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount, const VkColorComponentFlags *pColorWriteMasks);
static void __HINT_HCB_PFN_vkCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer, VkTessellationDomainOrigin domainOrigin);
static void __HINT_HCB_PFN_vkCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream);
static void __HINT_HCB_PFN_vkCmdSetConservativeRasterizationModeEXT(VkCommandBuffer commandBuffer, VkConservativeRasterizationModeEXT conservativeRasterizationMode);
static void __HINT_HCB_PFN_vkCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer, float extraPrimitiveOverestimationSize);
static void __HINT_HCB_PFN_vkCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable);
static void __HINT_HCB_PFN_vkCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer, VkBool32 sampleLocationsEnable);
static void __HINT_HCB_PFN_vkCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount, const VkColorBlendAdvancedEXT *pColorBlendAdvanced);
static void __HINT_HCB_PFN_vkCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer, VkProvokingVertexModeEXT provokingVertexMode);
static void __HINT_HCB_PFN_vkCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer, VkLineRasterizationModeEXT lineRasterizationMode);
static void __HINT_HCB_PFN_vkCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable);
static void __HINT_HCB_PFN_vkCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne);
static void __HINT_HCB_PFN_vkCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer, VkBool32 viewportWScalingEnable);
static void __HINT_HCB_PFN_vkCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewportSwizzleNV *pViewportSwizzles);
static void __HINT_HCB_PFN_vkCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageToColorEnable);
static void __HINT_HCB_PFN_vkCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer, uint32_t coverageToColorLocation);
static void __HINT_HCB_PFN_vkCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer, VkCoverageModulationModeNV coverageModulationMode);
static void __HINT_HCB_PFN_vkCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageModulationTableEnable);
static void __HINT_HCB_PFN_vkCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer, uint32_t coverageModulationTableCount, const float *pCoverageModulationTable);
static void __HINT_HCB_PFN_vkCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer, VkBool32 shadingRateImageEnable);
static void __HINT_HCB_PFN_vkCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer, VkBool32 representativeFragmentTestEnable);
static void __HINT_HCB_PFN_vkCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer, VkCoverageReductionModeNV coverageReductionMode);
static void __HINT_HCB_PFN_vkGetShaderModuleIdentifierEXT(VkDevice device, VkShaderModule shaderModule, VkShaderModuleIdentifierEXT *pIdentifier);
static void __HINT_HCB_PFN_vkGetShaderModuleCreateInfoIdentifierEXT(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo, VkShaderModuleIdentifierEXT *pIdentifier);
static VkResult __HINT_HCB_PFN_vkGetPhysicalDeviceOpticalFlowImageFormatsNV(VkPhysicalDevice physicalDevice, const VkOpticalFlowImageFormatInfoNV *pOpticalFlowImageFormatInfo, uint32_t *pFormatCount, VkOpticalFlowImageFormatPropertiesNV *pImageFormatProperties);
static VkResult __HINT_HCB_PFN_vkCreateOpticalFlowSessionNV(VkDevice device, const VkOpticalFlowSessionCreateInfoNV *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkOpticalFlowSessionNV *pSession);
static void __HINT_HCB_PFN_vkDestroyOpticalFlowSessionNV(VkDevice device, VkOpticalFlowSessionNV session, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_HCB_PFN_vkBindOpticalFlowSessionImageNV(VkDevice device, VkOpticalFlowSessionNV session, VkOpticalFlowSessionBindingPointNV bindingPoint, VkImageView view, VkImageLayout layout);
static void __HINT_HCB_PFN_vkCmdOpticalFlowExecuteNV(VkCommandBuffer commandBuffer, VkOpticalFlowSessionNV session, const VkOpticalFlowExecuteInfoNV *pExecuteInfo);
static void __HINT_HCB_PFN_vkAntiLagUpdateAMD(VkDevice device, const VkAntiLagDataAMD *pData);
static VkResult __HINT_HCB_PFN_vkCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders);
static void __HINT_HCB_PFN_vkDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_HCB_PFN_vkGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t *pDataSize, void *pData);
static void __HINT_HCB_PFN_vkCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount, const VkShaderStageFlagBits *pStages, const VkShaderEXT *pShaders);
static void __HINT_HCB_PFN_vkCmdSetDepthClampRangeEXT(VkCommandBuffer commandBuffer, VkDepthClampModeEXT depthClampMode, const VkDepthClampRangeEXT *pDepthClampRange);
static VkResult __HINT_HCB_PFN_vkGetFramebufferTilePropertiesQCOM(VkDevice device, VkFramebuffer framebuffer, uint32_t *pPropertiesCount, VkTilePropertiesQCOM *pProperties);
static VkResult __HINT_HCB_PFN_vkGetDynamicRenderingTilePropertiesQCOM(VkDevice device, const VkRenderingInfo *pRenderingInfo, VkTilePropertiesQCOM *pProperties);
static VkResult __HINT_HCB_PFN_vkGetPhysicalDeviceCooperativeVectorPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount, VkCooperativeVectorPropertiesNV *pProperties);
static VkResult __HINT_HCB_PFN_vkConvertCooperativeVectorMatrixNV(VkDevice device, const VkConvertCooperativeVectorMatrixInfoNV *pInfo);
static void __HINT_HCB_PFN_vkCmdConvertCooperativeVectorMatrixNV(VkCommandBuffer commandBuffer, uint32_t infoCount, const VkConvertCooperativeVectorMatrixInfoNV *pInfos);
static VkResult __HINT_HCB_PFN_vkSetLatencySleepModeNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepModeInfoNV *pSleepModeInfo);
static VkResult __HINT_HCB_PFN_vkLatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV *pSleepInfo);
static void __HINT_HCB_PFN_vkSetLatencyMarkerNV(VkDevice device, VkSwapchainKHR swapchain, const VkSetLatencyMarkerInfoNV *pLatencyMarkerInfo);
static void __HINT_HCB_PFN_vkGetLatencyTimingsNV(VkDevice device, VkSwapchainKHR swapchain, VkGetLatencyMarkerInfoNV *pLatencyMarkerInfo);
static void __HINT_HCB_PFN_vkQueueNotifyOutOfBandNV(VkQueue queue, const VkOutOfBandQueueTypeInfoNV *pQueueTypeInfo);
static void __HINT_HCB_PFN_vkCmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer commandBuffer, VkImageAspectFlags aspectMask);
static void __HINT_HCB_PFN_vkCmdBindTileMemoryQCOM(VkCommandBuffer commandBuffer, const VkTileMemoryBindInfoQCOM *pTileMemoryBindInfo);
static VkResult __HINT_HCB_PFN_vkCreateExternalComputeQueueNV(VkDevice device, const VkExternalComputeQueueCreateInfoNV *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkExternalComputeQueueNV *pExternalQueue);
static void __HINT_HCB_PFN_vkDestroyExternalComputeQueueNV(VkDevice device, VkExternalComputeQueueNV externalQueue, const VkAllocationCallbacks *pAllocator);
static void __HINT_HCB_PFN_vkGetExternalComputeQueueDataNV(VkExternalComputeQueueNV externalQueue, VkExternalComputeQueueDataParamsNV *params, void *pData);
static void __HINT_HCB_PFN_vkGetClusterAccelerationStructureBuildSizesNV(VkDevice device, const VkClusterAccelerationStructureInputInfoNV *pInfo, VkAccelerationStructureBuildSizesInfoKHR *pSizeInfo);
static void __HINT_HCB_PFN_vkCmdBuildClusterAccelerationStructureIndirectNV(VkCommandBuffer commandBuffer, const VkClusterAccelerationStructureCommandsInfoNV *pCommandInfos);
static void __HINT_HCB_PFN_vkGetPartitionedAccelerationStructuresBuildSizesNV(VkDevice device, const VkPartitionedAccelerationStructureInstancesInputNV *pInfo, VkAccelerationStructureBuildSizesInfoKHR *pSizeInfo);
static void __HINT_HCB_PFN_vkCmdBuildPartitionedAccelerationStructuresNV(VkCommandBuffer commandBuffer, const VkBuildPartitionedAccelerationStructureInfoNV *pBuildInfo);
static void __HINT_HCB_PFN_vkGetGeneratedCommandsMemoryRequirementsEXT(VkDevice device, const VkGeneratedCommandsMemoryRequirementsInfoEXT *pInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __HINT_HCB_PFN_vkCmdPreprocessGeneratedCommandsEXT(VkCommandBuffer commandBuffer, const VkGeneratedCommandsInfoEXT *pGeneratedCommandsInfo, VkCommandBuffer stateCommandBuffer);
static void __HINT_HCB_PFN_vkCmdExecuteGeneratedCommandsEXT(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed, const VkGeneratedCommandsInfoEXT *pGeneratedCommandsInfo);
static VkResult __HINT_HCB_PFN_vkCreateIndirectCommandsLayoutEXT(VkDevice device, const VkIndirectCommandsLayoutCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkIndirectCommandsLayoutEXT *pIndirectCommandsLayout);
static void __HINT_HCB_PFN_vkDestroyIndirectCommandsLayoutEXT(VkDevice device, VkIndirectCommandsLayoutEXT indirectCommandsLayout, const VkAllocationCallbacks *pAllocator);
static VkResult __HINT_HCB_PFN_vkCreateIndirectExecutionSetEXT(VkDevice device, const VkIndirectExecutionSetCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkIndirectExecutionSetEXT *pIndirectExecutionSet);
static void __HINT_HCB_PFN_vkDestroyIndirectExecutionSetEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet, const VkAllocationCallbacks *pAllocator);
static void __HINT_HCB_PFN_vkUpdateIndirectExecutionSetPipelineEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet, uint32_t executionSetWriteCount, const VkWriteIndirectExecutionSetPipelineEXT *pExecutionSetWrites);
static void __HINT_HCB_PFN_vkUpdateIndirectExecutionSetShaderEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet, uint32_t executionSetWriteCount, const VkWriteIndirectExecutionSetShaderEXT *pExecutionSetWrites);
static VkResult __HINT_HCB_PFN_vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount, VkCooperativeMatrixFlexibleDimensionsPropertiesNV *pProperties);
static void __HINT_HCB_PFN_vkCmdEndRendering2EXT(VkCommandBuffer commandBuffer, const VkRenderingEndInfoEXT *pRenderingEndInfo);
static VkResult __HINT_HCB_PFN_vkCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkAccelerationStructureKHR *pAccelerationStructure);
static void __HINT_HCB_PFN_vkDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure, const VkAllocationCallbacks *pAllocator);
static void __HINT_HCB_PFN_vkCmdBuildAccelerationStructuresKHR(VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos, const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos);
static void __HINT_HCB_PFN_vkCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos, const VkDeviceAddress *pIndirectDeviceAddresses, const uint32_t *pIndirectStrides, const uint32_t *const *ppMaxPrimitiveCounts);
static VkResult __HINT_HCB_PFN_vkBuildAccelerationStructuresKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos, const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos);
static VkResult __HINT_HCB_PFN_vkCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyAccelerationStructureInfoKHR *pInfo);
static VkResult __HINT_HCB_PFN_vkCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo);
static VkResult __HINT_HCB_PFN_vkCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo);
static VkResult __HINT_HCB_PFN_vkWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR *pAccelerationStructures, VkQueryType queryType, size_t dataSize, void *pData, size_t stride);
static void __HINT_HCB_PFN_vkCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureInfoKHR *pInfo);
static void __HINT_HCB_PFN_vkCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo);
static void __HINT_HCB_PFN_vkCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer, const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo);
static VkDeviceAddress __HINT_HCB_PFN_vkGetAccelerationStructureDeviceAddressKHR(VkDevice device, const VkAccelerationStructureDeviceAddressInfoKHR *pInfo);
static void __HINT_HCB_PFN_vkCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR *pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery);
static void __HINT_HCB_PFN_vkGetDeviceAccelerationStructureCompatibilityKHR(VkDevice device, const VkAccelerationStructureVersionInfoKHR *pVersionInfo, VkAccelerationStructureCompatibilityKHR *pCompatibility);
static void __HINT_HCB_PFN_vkGetAccelerationStructureBuildSizesKHR(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType, const VkAccelerationStructureBuildGeometryInfoKHR *pBuildInfo, const uint32_t *pMaxPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR *pSizeInfo);
static void __HINT_HCB_PFN_vkCmdTraceRaysKHR(VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable, const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable, const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable, const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable, uint32_t width, uint32_t height, uint32_t depth);
static VkResult __HINT_HCB_PFN_vkCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines);
static VkResult __HINT_HCB_PFN_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void *pData);
static void __HINT_HCB_PFN_vkCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable, const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable, const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable, const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable, VkDeviceAddress indirectDeviceAddress);
static VkDeviceSize __HINT_HCB_PFN_vkGetRayTracingShaderGroupStackSizeKHR(VkDevice device, VkPipeline pipeline, uint32_t group, VkShaderGroupShaderKHR groupShader);
static void __HINT_HCB_PFN_vkCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer, uint32_t pipelineStackSize);
static void __HINT_HCB_PFN_vkCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
static void __HINT_HCB_PFN_vkCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
static void __HINT_HCB_PFN_vkCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);
static VkResult __HINT_HCB_PFN_vkCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface);
static VkBool32 __HINT_HCB_PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, struct wl_display *display);
static VkResult __HINT_HCB_PFN_vkCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface);
static VkBool32 __HINT_HCB_PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, xcb_connection_t *connection, xcb_visualid_t visual_id);
static VkResult __HINT_HCB_PFN_vkCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface);
static VkBool32 __HINT_HCB_PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, Display *dpy, VisualID visualID);

#if defined(LORELIB_GTL_BUILD) || defined(LORELIB_VISUAL)

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundefined-internal"

// GTPs
static VkResult __GTP_HCB_PFN_vkCreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkInstance *pInstance);
static void __GTP_HCB_PFN_vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator);
static VkResult __GTP_HCB_PFN_vkEnumeratePhysicalDevices(VkInstance instance, uint32_t *pPhysicalDeviceCount, VkPhysicalDevice *pPhysicalDevices);
static void __GTP_HCB_PFN_vkGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures *pFeatures);
static void __GTP_HCB_PFN_vkGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties *pFormatProperties);
static VkResult __GTP_HCB_PFN_vkGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkImageFormatProperties *pImageFormatProperties);
static void __GTP_HCB_PFN_vkGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties *pProperties);
static void __GTP_HCB_PFN_vkGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t *pQueueFamilyPropertyCount, VkQueueFamilyProperties *pQueueFamilyProperties);
static void __GTP_HCB_PFN_vkGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties *pMemoryProperties);
static PFN_vkVoidFunction __GTP_HCB_PFN_vkGetInstanceProcAddr(VkInstance instance, const char *pName);
static PFN_vkVoidFunction __GTP_HCB_PFN_vkGetDeviceProcAddr(VkDevice device, const char *pName);
static VkResult __GTP_HCB_PFN_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDevice *pDevice);
static void __GTP_HCB_PFN_vkDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator);
static VkResult __GTP_HCB_PFN_vkEnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pPropertyCount, VkExtensionProperties *pProperties);
static VkResult __GTP_HCB_PFN_vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char *pLayerName, uint32_t *pPropertyCount, VkExtensionProperties *pProperties);
static VkResult __GTP_HCB_PFN_vkEnumerateInstanceLayerProperties(uint32_t *pPropertyCount, VkLayerProperties *pProperties);
static VkResult __GTP_HCB_PFN_vkEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount, VkLayerProperties *pProperties);
static void __GTP_HCB_PFN_vkGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue *pQueue);
static VkResult __GTP_HCB_PFN_vkQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence);
static VkResult __GTP_HCB_PFN_vkQueueWaitIdle(VkQueue queue);
static VkResult __GTP_HCB_PFN_vkDeviceWaitIdle(VkDevice device);
static VkResult __GTP_HCB_PFN_vkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo, const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory);
static void __GTP_HCB_PFN_vkFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks *pAllocator);
static VkResult __GTP_HCB_PFN_vkMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void **ppData);
static void __GTP_HCB_PFN_vkUnmapMemory(VkDevice device, VkDeviceMemory memory);
static VkResult __GTP_HCB_PFN_vkFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange *pMemoryRanges);
static VkResult __GTP_HCB_PFN_vkInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange *pMemoryRanges);
static void __GTP_HCB_PFN_vkGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize *pCommittedMemoryInBytes);
static VkResult __GTP_HCB_PFN_vkBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset);
static VkResult __GTP_HCB_PFN_vkBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset);
static void __GTP_HCB_PFN_vkGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements *pMemoryRequirements);
static void __GTP_HCB_PFN_vkGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements *pMemoryRequirements);
static void __GTP_HCB_PFN_vkGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t *pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements *pSparseMemoryRequirements);
static void __GTP_HCB_PFN_vkGetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t *pPropertyCount, VkSparseImageFormatProperties *pProperties);
static VkResult __GTP_HCB_PFN_vkQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo *pBindInfo, VkFence fence);
static VkResult __GTP_HCB_PFN_vkCreateFence(VkDevice device, const VkFenceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkFence *pFence);
static void __GTP_HCB_PFN_vkDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks *pAllocator);
static VkResult __GTP_HCB_PFN_vkResetFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences);
static VkResult __GTP_HCB_PFN_vkGetFenceStatus(VkDevice device, VkFence fence);
static VkResult __GTP_HCB_PFN_vkWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences, VkBool32 waitAll, uint64_t timeout);
static VkResult __GTP_HCB_PFN_vkCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSemaphore *pSemaphore);
static void __GTP_HCB_PFN_vkDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks *pAllocator);
static VkResult __GTP_HCB_PFN_vkCreateEvent(VkDevice device, const VkEventCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkEvent *pEvent);
static void __GTP_HCB_PFN_vkDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks *pAllocator);
static VkResult __GTP_HCB_PFN_vkGetEventStatus(VkDevice device, VkEvent event);
static VkResult __GTP_HCB_PFN_vkSetEvent(VkDevice device, VkEvent event);
static VkResult __GTP_HCB_PFN_vkResetEvent(VkDevice device, VkEvent event);
static VkResult __GTP_HCB_PFN_vkCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkQueryPool *pQueryPool);
static void __GTP_HCB_PFN_vkDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks *pAllocator);
static VkResult __GTP_HCB_PFN_vkGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void *pData, VkDeviceSize stride, VkQueryResultFlags flags);
static VkResult __GTP_HCB_PFN_vkCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer);
static void __GTP_HCB_PFN_vkDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks *pAllocator);
static VkResult __GTP_HCB_PFN_vkCreateBufferView(VkDevice device, const VkBufferViewCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkBufferView *pView);
static void __GTP_HCB_PFN_vkDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks *pAllocator);
static VkResult __GTP_HCB_PFN_vkCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkImage *pImage);
static void __GTP_HCB_PFN_vkDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator);
static void __GTP_HCB_PFN_vkGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource *pSubresource, VkSubresourceLayout *pLayout);
static VkResult __GTP_HCB_PFN_vkCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkImageView *pView);
static void __GTP_HCB_PFN_vkDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks *pAllocator);
static VkResult __GTP_HCB_PFN_vkCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkShaderModule *pShaderModule);
static void __GTP_HCB_PFN_vkDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks *pAllocator);
static VkResult __GTP_HCB_PFN_vkCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkPipelineCache *pPipelineCache);
static void __GTP_HCB_PFN_vkDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks *pAllocator);
static VkResult __GTP_HCB_PFN_vkGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t *pDataSize, void *pData);
static VkResult __GTP_HCB_PFN_vkMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount, const VkPipelineCache *pSrcCaches);
static VkResult __GTP_HCB_PFN_vkCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines);
static VkResult __GTP_HCB_PFN_vkCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines);
static void __GTP_HCB_PFN_vkDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks *pAllocator);
static VkResult __GTP_HCB_PFN_vkCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout);
static void __GTP_HCB_PFN_vkDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks *pAllocator);
static VkResult __GTP_HCB_PFN_vkCreateSampler(VkDevice device, const VkSamplerCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSampler *pSampler);
static void __GTP_HCB_PFN_vkDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks *pAllocator);
static VkResult __GTP_HCB_PFN_vkCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDescriptorSetLayout *pSetLayout);
static void __GTP_HCB_PFN_vkDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks *pAllocator);
static VkResult __GTP_HCB_PFN_vkCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDescriptorPool *pDescriptorPool);
static void __GTP_HCB_PFN_vkDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks *pAllocator);
static VkResult __GTP_HCB_PFN_vkResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags);
static VkResult __GTP_HCB_PFN_vkAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo *pAllocateInfo, VkDescriptorSet *pDescriptorSets);
static VkResult __GTP_HCB_PFN_vkFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet *pDescriptorSets);
static void __GTP_HCB_PFN_vkUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount, const VkWriteDescriptorSet *pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet *pDescriptorCopies);
static VkResult __GTP_HCB_PFN_vkCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkFramebuffer *pFramebuffer);
static void __GTP_HCB_PFN_vkDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks *pAllocator);
static VkResult __GTP_HCB_PFN_vkCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass);
static void __GTP_HCB_PFN_vkDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks *pAllocator);
static void __GTP_HCB_PFN_vkGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D *pGranularity);
static VkResult __GTP_HCB_PFN_vkCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkCommandPool *pCommandPool);
static void __GTP_HCB_PFN_vkDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks *pAllocator);
static VkResult __GTP_HCB_PFN_vkResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags);
static VkResult __GTP_HCB_PFN_vkAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo *pAllocateInfo, VkCommandBuffer *pCommandBuffers);
static void __GTP_HCB_PFN_vkFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer *pCommandBuffers);
static VkResult __GTP_HCB_PFN_vkBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *pBeginInfo);
static VkResult __GTP_HCB_PFN_vkEndCommandBuffer(VkCommandBuffer commandBuffer);
static VkResult __GTP_HCB_PFN_vkResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags);
static void __GTP_HCB_PFN_vkCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);
static void __GTP_HCB_PFN_vkCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport *pViewports);
static void __GTP_HCB_PFN_vkCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D *pScissors);
static void __GTP_HCB_PFN_vkCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth);
static void __GTP_HCB_PFN_vkCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor);
static void __GTP_HCB_PFN_vkCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]);
static void __GTP_HCB_PFN_vkCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds);
static void __GTP_HCB_PFN_vkCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask);
static void __GTP_HCB_PFN_vkCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask);
static void __GTP_HCB_PFN_vkCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference);
static void __GTP_HCB_PFN_vkCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet *pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t *pDynamicOffsets);
static void __GTP_HCB_PFN_vkCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType);
static void __GTP_HCB_PFN_vkCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer *pBuffers, const VkDeviceSize *pOffsets);
static void __GTP_HCB_PFN_vkCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
static void __GTP_HCB_PFN_vkCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
static void __GTP_HCB_PFN_vkCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
static void __GTP_HCB_PFN_vkCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
static void __GTP_HCB_PFN_vkCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
static void __GTP_HCB_PFN_vkCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset);
static void __GTP_HCB_PFN_vkCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy *pRegions);
static void __GTP_HCB_PFN_vkCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy *pRegions);
static void __GTP_HCB_PFN_vkCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit *pRegions, VkFilter filter);
static void __GTP_HCB_PFN_vkCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy *pRegions);
static void __GTP_HCB_PFN_vkCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions);
static void __GTP_HCB_PFN_vkCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void *pData);
static void __GTP_HCB_PFN_vkCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data);
static void __GTP_HCB_PFN_vkCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue *pColor, uint32_t rangeCount, const VkImageSubresourceRange *pRanges);
static void __GTP_HCB_PFN_vkCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange *pRanges);
static void __GTP_HCB_PFN_vkCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment *pAttachments, uint32_t rectCount, const VkClearRect *pRects);
static void __GTP_HCB_PFN_vkCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve *pRegions);
static void __GTP_HCB_PFN_vkCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask);
static void __GTP_HCB_PFN_vkCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask);
static void __GTP_HCB_PFN_vkCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers);
static void __GTP_HCB_PFN_vkCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers);
static void __GTP_HCB_PFN_vkCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags);
static void __GTP_HCB_PFN_vkCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query);
static void __GTP_HCB_PFN_vkCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);
static void __GTP_HCB_PFN_vkCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query);
static void __GTP_HCB_PFN_vkCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags);
static void __GTP_HCB_PFN_vkCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void *pValues);
static void __GTP_HCB_PFN_vkCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin, VkSubpassContents contents);
static void __GTP_HCB_PFN_vkCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents);
static void __GTP_HCB_PFN_vkCmdEndRenderPass(VkCommandBuffer commandBuffer);
static void __GTP_HCB_PFN_vkCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount, const VkCommandBuffer *pCommandBuffers);
static VkResult __GTP_HCB_PFN_vkEnumerateInstanceVersion(uint32_t *pApiVersion);
static VkResult __GTP_HCB_PFN_vkBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo *pBindInfos);
static VkResult __GTP_HCB_PFN_vkBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo *pBindInfos);
static void __GTP_HCB_PFN_vkGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags *pPeerMemoryFeatures);
static void __GTP_HCB_PFN_vkCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask);
static void __GTP_HCB_PFN_vkCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
static VkResult __GTP_HCB_PFN_vkEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t *pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroupProperties);
static void __GTP_HCB_PFN_vkGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2 *pInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __GTP_HCB_PFN_vkGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2 *pInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __GTP_HCB_PFN_vkGetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2 *pInfo, uint32_t *pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements);
static void __GTP_HCB_PFN_vkGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2 *pFeatures);
static void __GTP_HCB_PFN_vkGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2 *pProperties);
static void __GTP_HCB_PFN_vkGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2 *pFormatProperties);
static VkResult __GTP_HCB_PFN_vkGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo, VkImageFormatProperties2 *pImageFormatProperties);
static void __GTP_HCB_PFN_vkGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t *pQueueFamilyPropertyCount, VkQueueFamilyProperties2 *pQueueFamilyProperties);
static void __GTP_HCB_PFN_vkGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2 *pMemoryProperties);
static void __GTP_HCB_PFN_vkGetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2 *pFormatInfo, uint32_t *pPropertyCount, VkSparseImageFormatProperties2 *pProperties);
static void __GTP_HCB_PFN_vkTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags);
static void __GTP_HCB_PFN_vkGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2 *pQueueInfo, VkQueue *pQueue);
static VkResult __GTP_HCB_PFN_vkCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSamplerYcbcrConversion *pYcbcrConversion);
static void __GTP_HCB_PFN_vkDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks *pAllocator);
static VkResult __GTP_HCB_PFN_vkCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate);
static void __GTP_HCB_PFN_vkDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks *pAllocator);
static void __GTP_HCB_PFN_vkUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void *pData);
static void __GTP_HCB_PFN_vkGetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo *pExternalBufferInfo, VkExternalBufferProperties *pExternalBufferProperties);
static void __GTP_HCB_PFN_vkGetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo *pExternalFenceInfo, VkExternalFenceProperties *pExternalFenceProperties);
static void __GTP_HCB_PFN_vkGetPhysicalDeviceExternalSemaphoreProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo *pExternalSemaphoreInfo, VkExternalSemaphoreProperties *pExternalSemaphoreProperties);
static void __GTP_HCB_PFN_vkGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo, VkDescriptorSetLayoutSupport *pSupport);
static void __GTP_HCB_PFN_vkCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);
static void __GTP_HCB_PFN_vkCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);
static VkResult __GTP_HCB_PFN_vkCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass);
static void __GTP_HCB_PFN_vkCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin, const VkSubpassBeginInfo *pSubpassBeginInfo);
static void __GTP_HCB_PFN_vkCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo, const VkSubpassEndInfo *pSubpassEndInfo);
static void __GTP_HCB_PFN_vkCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo);
static void __GTP_HCB_PFN_vkResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);
static VkResult __GTP_HCB_PFN_vkGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t *pValue);
static VkResult __GTP_HCB_PFN_vkWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout);
static VkResult __GTP_HCB_PFN_vkSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo);
static VkDeviceAddress __GTP_HCB_PFN_vkGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo *pInfo);
static uint64_t __GTP_HCB_PFN_vkGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo *pInfo);
static uint64_t __GTP_HCB_PFN_vkGetDeviceMemoryOpaqueCaptureAddress(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo *pInfo);
static VkResult __GTP_HCB_PFN_vkGetPhysicalDeviceToolProperties(VkPhysicalDevice physicalDevice, uint32_t *pToolCount, VkPhysicalDeviceToolProperties *pToolProperties);
static VkResult __GTP_HCB_PFN_vkCreatePrivateDataSlot(VkDevice device, const VkPrivateDataSlotCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkPrivateDataSlot *pPrivateDataSlot);
static void __GTP_HCB_PFN_vkDestroyPrivateDataSlot(VkDevice device, VkPrivateDataSlot privateDataSlot, const VkAllocationCallbacks *pAllocator);
static VkResult __GTP_HCB_PFN_vkSetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlot privateDataSlot, uint64_t data);
static void __GTP_HCB_PFN_vkGetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlot privateDataSlot, uint64_t *pData);
static void __GTP_HCB_PFN_vkCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo *pDependencyInfo);
static void __GTP_HCB_PFN_vkCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask);
static void __GTP_HCB_PFN_vkCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents, const VkDependencyInfo *pDependencyInfos);
static void __GTP_HCB_PFN_vkCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo);
static void __GTP_HCB_PFN_vkCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool, uint32_t query);
static VkResult __GTP_HCB_PFN_vkQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits, VkFence fence);
static void __GTP_HCB_PFN_vkCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo);
static void __GTP_HCB_PFN_vkCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo);
static void __GTP_HCB_PFN_vkCmdCopyBufferToImage2(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo);
static void __GTP_HCB_PFN_vkCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo);
static void __GTP_HCB_PFN_vkCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2 *pBlitImageInfo);
static void __GTP_HCB_PFN_vkCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2 *pResolveImageInfo);
static void __GTP_HCB_PFN_vkCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo);
static void __GTP_HCB_PFN_vkCmdEndRendering(VkCommandBuffer commandBuffer);
static void __GTP_HCB_PFN_vkCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode);
static void __GTP_HCB_PFN_vkCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace);
static void __GTP_HCB_PFN_vkCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology);
static void __GTP_HCB_PFN_vkCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport *pViewports);
static void __GTP_HCB_PFN_vkCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D *pScissors);
static void __GTP_HCB_PFN_vkCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer *pBuffers, const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes, const VkDeviceSize *pStrides);
static void __GTP_HCB_PFN_vkCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable);
static void __GTP_HCB_PFN_vkCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable);
static void __GTP_HCB_PFN_vkCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp);
static void __GTP_HCB_PFN_vkCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable);
static void __GTP_HCB_PFN_vkCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable);
static void __GTP_HCB_PFN_vkCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp);
static void __GTP_HCB_PFN_vkCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable);
static void __GTP_HCB_PFN_vkCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable);
static void __GTP_HCB_PFN_vkCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable);
static void __GTP_HCB_PFN_vkGetDeviceBufferMemoryRequirements(VkDevice device, const VkDeviceBufferMemoryRequirements *pInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __GTP_HCB_PFN_vkGetDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements *pInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __GTP_HCB_PFN_vkGetDeviceImageSparseMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements *pInfo, uint32_t *pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements);
static void __GTP_HCB_PFN_vkCmdSetLineStipple(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern);
static VkResult __GTP_HCB_PFN_vkMapMemory2(VkDevice device, const VkMemoryMapInfo *pMemoryMapInfo, void **ppData);
static VkResult __GTP_HCB_PFN_vkUnmapMemory2(VkDevice device, const VkMemoryUnmapInfo *pMemoryUnmapInfo);
static void __GTP_HCB_PFN_vkCmdBindIndexBuffer2(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size, VkIndexType indexType);
static void __GTP_HCB_PFN_vkGetRenderingAreaGranularity(VkDevice device, const VkRenderingAreaInfo *pRenderingAreaInfo, VkExtent2D *pGranularity);
static void __GTP_HCB_PFN_vkGetDeviceImageSubresourceLayout(VkDevice device, const VkDeviceImageSubresourceInfo *pInfo, VkSubresourceLayout2 *pLayout);
static void __GTP_HCB_PFN_vkGetImageSubresourceLayout2(VkDevice device, VkImage image, const VkImageSubresource2 *pSubresource, VkSubresourceLayout2 *pLayout);
static void __GTP_HCB_PFN_vkCmdPushDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount, const VkWriteDescriptorSet *pDescriptorWrites);
static void __GTP_HCB_PFN_vkCmdPushDescriptorSetWithTemplate(VkCommandBuffer commandBuffer, VkDescriptorUpdateTemplate descriptorUpdateTemplate, VkPipelineLayout layout, uint32_t set, const void *pData);
static void __GTP_HCB_PFN_vkCmdSetRenderingAttachmentLocations(VkCommandBuffer commandBuffer, const VkRenderingAttachmentLocationInfo *pLocationInfo);
static void __GTP_HCB_PFN_vkCmdSetRenderingInputAttachmentIndices(VkCommandBuffer commandBuffer, const VkRenderingInputAttachmentIndexInfo *pInputAttachmentIndexInfo);
static void __GTP_HCB_PFN_vkCmdBindDescriptorSets2(VkCommandBuffer commandBuffer, const VkBindDescriptorSetsInfo *pBindDescriptorSetsInfo);
static void __GTP_HCB_PFN_vkCmdPushConstants2(VkCommandBuffer commandBuffer, const VkPushConstantsInfo *pPushConstantsInfo);
static void __GTP_HCB_PFN_vkCmdPushDescriptorSet2(VkCommandBuffer commandBuffer, const VkPushDescriptorSetInfo *pPushDescriptorSetInfo);
static void __GTP_HCB_PFN_vkCmdPushDescriptorSetWithTemplate2(VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfo *pPushDescriptorSetWithTemplateInfo);
static VkResult __GTP_HCB_PFN_vkCopyMemoryToImage(VkDevice device, const VkCopyMemoryToImageInfo *pCopyMemoryToImageInfo);
static VkResult __GTP_HCB_PFN_vkCopyImageToMemory(VkDevice device, const VkCopyImageToMemoryInfo *pCopyImageToMemoryInfo);
static VkResult __GTP_HCB_PFN_vkCopyImageToImage(VkDevice device, const VkCopyImageToImageInfo *pCopyImageToImageInfo);
static VkResult __GTP_HCB_PFN_vkTransitionImageLayout(VkDevice device, uint32_t transitionCount, const VkHostImageLayoutTransitionInfo *pTransitions);
static void __GTP_HCB_PFN_vkDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks *pAllocator);
static VkResult __GTP_HCB_PFN_vkGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32 *pSupported);
static VkResult __GTP_HCB_PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR *pSurfaceCapabilities);
static VkResult __GTP_HCB_PFN_vkGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t *pSurfaceFormatCount, VkSurfaceFormatKHR *pSurfaceFormats);
static VkResult __GTP_HCB_PFN_vkGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t *pPresentModeCount, VkPresentModeKHR *pPresentModes);
static VkResult __GTP_HCB_PFN_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchain);
static void __GTP_HCB_PFN_vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks *pAllocator);
static VkResult __GTP_HCB_PFN_vkGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t *pSwapchainImageCount, VkImage *pSwapchainImages);
static VkResult __GTP_HCB_PFN_vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex);
static VkResult __GTP_HCB_PFN_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo);
static VkResult __GTP_HCB_PFN_vkGetDeviceGroupPresentCapabilitiesKHR(VkDevice device, VkDeviceGroupPresentCapabilitiesKHR *pDeviceGroupPresentCapabilities);
static VkResult __GTP_HCB_PFN_vkGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface, VkDeviceGroupPresentModeFlagsKHR *pModes);
static VkResult __GTP_HCB_PFN_vkGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t *pRectCount, VkRect2D *pRects);
static VkResult __GTP_HCB_PFN_vkAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR *pAcquireInfo, uint32_t *pImageIndex);
static VkResult __GTP_HCB_PFN_vkGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount, VkDisplayPropertiesKHR *pProperties);
static VkResult __GTP_HCB_PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount, VkDisplayPlanePropertiesKHR *pProperties);
static VkResult __GTP_HCB_PFN_vkGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex, uint32_t *pDisplayCount, VkDisplayKHR *pDisplays);
static VkResult __GTP_HCB_PFN_vkGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t *pPropertyCount, VkDisplayModePropertiesKHR *pProperties);
static VkResult __GTP_HCB_PFN_vkCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, const VkDisplayModeCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDisplayModeKHR *pMode);
static VkResult __GTP_HCB_PFN_vkGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR *pCapabilities);
static VkResult __GTP_HCB_PFN_vkCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface);
static VkResult __GTP_HCB_PFN_vkCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount, const VkSwapchainCreateInfoKHR *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchains);
static VkResult __GTP_HCB_PFN_vkGetPhysicalDeviceVideoCapabilitiesKHR(VkPhysicalDevice physicalDevice, const VkVideoProfileInfoKHR *pVideoProfile, VkVideoCapabilitiesKHR *pCapabilities);
static VkResult __GTP_HCB_PFN_vkGetPhysicalDeviceVideoFormatPropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoFormatInfoKHR *pVideoFormatInfo, uint32_t *pVideoFormatPropertyCount, VkVideoFormatPropertiesKHR *pVideoFormatProperties);
static VkResult __GTP_HCB_PFN_vkCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkVideoSessionKHR *pVideoSession);
static void __GTP_HCB_PFN_vkDestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession, const VkAllocationCallbacks *pAllocator);
static VkResult __GTP_HCB_PFN_vkGetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession, uint32_t *pMemoryRequirementsCount, VkVideoSessionMemoryRequirementsKHR *pMemoryRequirements);
static VkResult __GTP_HCB_PFN_vkBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession, uint32_t bindSessionMemoryInfoCount, const VkBindVideoSessionMemoryInfoKHR *pBindSessionMemoryInfos);
static VkResult __GTP_HCB_PFN_vkCreateVideoSessionParametersKHR(VkDevice device, const VkVideoSessionParametersCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkVideoSessionParametersKHR *pVideoSessionParameters);
static VkResult __GTP_HCB_PFN_vkUpdateVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters, const VkVideoSessionParametersUpdateInfoKHR *pUpdateInfo);
static void __GTP_HCB_PFN_vkDestroyVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters, const VkAllocationCallbacks *pAllocator);
static void __GTP_HCB_PFN_vkCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR *pBeginInfo);
static void __GTP_HCB_PFN_vkCmdEndVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoEndCodingInfoKHR *pEndCodingInfo);
static void __GTP_HCB_PFN_vkCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoCodingControlInfoKHR *pCodingControlInfo);
static void __GTP_HCB_PFN_vkCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR *pDecodeInfo);
static void __GTP_HCB_PFN_vkCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo);
static void __GTP_HCB_PFN_vkCmdEndRenderingKHR(VkCommandBuffer commandBuffer);
static void __GTP_HCB_PFN_vkGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2 *pFeatures);
static void __GTP_HCB_PFN_vkGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2 *pProperties);
static void __GTP_HCB_PFN_vkGetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2 *pFormatProperties);
static VkResult __GTP_HCB_PFN_vkGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo, VkImageFormatProperties2 *pImageFormatProperties);
static void __GTP_HCB_PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t *pQueueFamilyPropertyCount, VkQueueFamilyProperties2 *pQueueFamilyProperties);
static void __GTP_HCB_PFN_vkGetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2 *pMemoryProperties);
static void __GTP_HCB_PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2 *pFormatInfo, uint32_t *pPropertyCount, VkSparseImageFormatProperties2 *pProperties);
static void __GTP_HCB_PFN_vkGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags *pPeerMemoryFeatures);
static void __GTP_HCB_PFN_vkCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask);
static void __GTP_HCB_PFN_vkCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
static void __GTP_HCB_PFN_vkTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags);
static VkResult __GTP_HCB_PFN_vkEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t *pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroupProperties);
static void __GTP_HCB_PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo *pExternalBufferInfo, VkExternalBufferProperties *pExternalBufferProperties);
static VkResult __GTP_HCB_PFN_vkGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR *pGetFdInfo, int *pFd);
static VkResult __GTP_HCB_PFN_vkGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd, VkMemoryFdPropertiesKHR *pMemoryFdProperties);
static void __GTP_HCB_PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo *pExternalSemaphoreInfo, VkExternalSemaphoreProperties *pExternalSemaphoreProperties);
static VkResult __GTP_HCB_PFN_vkImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR *pImportSemaphoreFdInfo);
static VkResult __GTP_HCB_PFN_vkGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR *pGetFdInfo, int *pFd);
static void __GTP_HCB_PFN_vkCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount, const VkWriteDescriptorSet *pDescriptorWrites);
static void __GTP_HCB_PFN_vkCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer, VkDescriptorUpdateTemplate descriptorUpdateTemplate, VkPipelineLayout layout, uint32_t set, const void *pData);
static VkResult __GTP_HCB_PFN_vkCreateDescriptorUpdateTemplateKHR(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate);
static void __GTP_HCB_PFN_vkDestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks *pAllocator);
static void __GTP_HCB_PFN_vkUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void *pData);
static VkResult __GTP_HCB_PFN_vkCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass);
static void __GTP_HCB_PFN_vkCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin, const VkSubpassBeginInfo *pSubpassBeginInfo);
static void __GTP_HCB_PFN_vkCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo, const VkSubpassEndInfo *pSubpassEndInfo);
static void __GTP_HCB_PFN_vkCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo);
static VkResult __GTP_HCB_PFN_vkGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain);
static void __GTP_HCB_PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo *pExternalFenceInfo, VkExternalFenceProperties *pExternalFenceProperties);
static VkResult __GTP_HCB_PFN_vkImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR *pImportFenceFdInfo);
static VkResult __GTP_HCB_PFN_vkGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR *pGetFdInfo, int *pFd);
static VkResult __GTP_HCB_PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t *pCounterCount, VkPerformanceCounterKHR *pCounters, VkPerformanceCounterDescriptionKHR *pCounterDescriptions);
static void __GTP_HCB_PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(VkPhysicalDevice physicalDevice, const VkQueryPoolPerformanceCreateInfoKHR *pPerformanceQueryCreateInfo, uint32_t *pNumPasses);
static VkResult __GTP_HCB_PFN_vkAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR *pInfo);
static void __GTP_HCB_PFN_vkReleaseProfilingLockKHR(VkDevice device);
static VkResult __GTP_HCB_PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo, VkSurfaceCapabilities2KHR *pSurfaceCapabilities);
static VkResult __GTP_HCB_PFN_vkGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo, uint32_t *pSurfaceFormatCount, VkSurfaceFormat2KHR *pSurfaceFormats);
static VkResult __GTP_HCB_PFN_vkGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount, VkDisplayProperties2KHR *pProperties);
static VkResult __GTP_HCB_PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount, VkDisplayPlaneProperties2KHR *pProperties);
static VkResult __GTP_HCB_PFN_vkGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t *pPropertyCount, VkDisplayModeProperties2KHR *pProperties);
static VkResult __GTP_HCB_PFN_vkGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice, const VkDisplayPlaneInfo2KHR *pDisplayPlaneInfo, VkDisplayPlaneCapabilities2KHR *pCapabilities);
static void __GTP_HCB_PFN_vkGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2 *pInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __GTP_HCB_PFN_vkGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2 *pInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __GTP_HCB_PFN_vkGetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2 *pInfo, uint32_t *pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements);
static VkResult __GTP_HCB_PFN_vkCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSamplerYcbcrConversion *pYcbcrConversion);
static void __GTP_HCB_PFN_vkDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks *pAllocator);
static VkResult __GTP_HCB_PFN_vkBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo *pBindInfos);
static VkResult __GTP_HCB_PFN_vkBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo *pBindInfos);
static void __GTP_HCB_PFN_vkGetDescriptorSetLayoutSupportKHR(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo, VkDescriptorSetLayoutSupport *pSupport);
static void __GTP_HCB_PFN_vkCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);
static void __GTP_HCB_PFN_vkCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);
static VkResult __GTP_HCB_PFN_vkGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t *pValue);
static VkResult __GTP_HCB_PFN_vkWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout);
static VkResult __GTP_HCB_PFN_vkSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo);
static VkResult __GTP_HCB_PFN_vkGetPhysicalDeviceFragmentShadingRatesKHR(VkPhysicalDevice physicalDevice, uint32_t *pFragmentShadingRateCount, VkPhysicalDeviceFragmentShadingRateKHR *pFragmentShadingRates);
static void __GTP_HCB_PFN_vkCmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer, const VkExtent2D *pFragmentSize, const VkFragmentShadingRateCombinerOpKHR combinerOps[2]);
static void __GTP_HCB_PFN_vkCmdSetRenderingAttachmentLocationsKHR(VkCommandBuffer commandBuffer, const VkRenderingAttachmentLocationInfo *pLocationInfo);
static void __GTP_HCB_PFN_vkCmdSetRenderingInputAttachmentIndicesKHR(VkCommandBuffer commandBuffer, const VkRenderingInputAttachmentIndexInfo *pInputAttachmentIndexInfo);
static VkResult __GTP_HCB_PFN_vkWaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId, uint64_t timeout);
static VkDeviceAddress __GTP_HCB_PFN_vkGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo *pInfo);
static uint64_t __GTP_HCB_PFN_vkGetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo *pInfo);
static uint64_t __GTP_HCB_PFN_vkGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo *pInfo);
static VkResult __GTP_HCB_PFN_vkCreateDeferredOperationKHR(VkDevice device, const VkAllocationCallbacks *pAllocator, VkDeferredOperationKHR *pDeferredOperation);
static void __GTP_HCB_PFN_vkDestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation, const VkAllocationCallbacks *pAllocator);
static uint32_t __GTP_HCB_PFN_vkGetDeferredOperationMaxConcurrencyKHR(VkDevice device, VkDeferredOperationKHR operation);
static VkResult __GTP_HCB_PFN_vkGetDeferredOperationResultKHR(VkDevice device, VkDeferredOperationKHR operation);
static VkResult __GTP_HCB_PFN_vkDeferredOperationJoinKHR(VkDevice device, VkDeferredOperationKHR operation);
static VkResult __GTP_HCB_PFN_vkGetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR *pPipelineInfo, uint32_t *pExecutableCount, VkPipelineExecutablePropertiesKHR *pProperties);
static VkResult __GTP_HCB_PFN_vkGetPipelineExecutableStatisticsKHR(VkDevice device, const VkPipelineExecutableInfoKHR *pExecutableInfo, uint32_t *pStatisticCount, VkPipelineExecutableStatisticKHR *pStatistics);
static VkResult __GTP_HCB_PFN_vkGetPipelineExecutableInternalRepresentationsKHR(VkDevice device, const VkPipelineExecutableInfoKHR *pExecutableInfo, uint32_t *pInternalRepresentationCount, VkPipelineExecutableInternalRepresentationKHR *pInternalRepresentations);
static VkResult __GTP_HCB_PFN_vkMapMemory2KHR(VkDevice device, const VkMemoryMapInfo *pMemoryMapInfo, void **ppData);
static VkResult __GTP_HCB_PFN_vkUnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfo *pMemoryUnmapInfo);
static VkResult __GTP_HCB_PFN_vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR *pQualityLevelInfo, VkVideoEncodeQualityLevelPropertiesKHR *pQualityLevelProperties);
static VkResult __GTP_HCB_PFN_vkGetEncodedVideoSessionParametersKHR(VkDevice device, const VkVideoEncodeSessionParametersGetInfoKHR *pVideoSessionParametersInfo, VkVideoEncodeSessionParametersFeedbackInfoKHR *pFeedbackInfo, size_t *pDataSize, void *pData);
static void __GTP_HCB_PFN_vkCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR *pEncodeInfo);
static void __GTP_HCB_PFN_vkCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo *pDependencyInfo);
static void __GTP_HCB_PFN_vkCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask);
static void __GTP_HCB_PFN_vkCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents, const VkDependencyInfo *pDependencyInfos);
static void __GTP_HCB_PFN_vkCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo);
static void __GTP_HCB_PFN_vkCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool, uint32_t query);
static VkResult __GTP_HCB_PFN_vkQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits, VkFence fence);
static void __GTP_HCB_PFN_vkCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo);
static void __GTP_HCB_PFN_vkCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo);
static void __GTP_HCB_PFN_vkCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo);
static void __GTP_HCB_PFN_vkCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo);
static void __GTP_HCB_PFN_vkCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2 *pBlitImageInfo);
static void __GTP_HCB_PFN_vkCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2 *pResolveImageInfo);
static void __GTP_HCB_PFN_vkCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress);
static void __GTP_HCB_PFN_vkGetDeviceBufferMemoryRequirementsKHR(VkDevice device, const VkDeviceBufferMemoryRequirements *pInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __GTP_HCB_PFN_vkGetDeviceImageMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements *pInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __GTP_HCB_PFN_vkGetDeviceImageSparseMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements *pInfo, uint32_t *pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements);
static void __GTP_HCB_PFN_vkCmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size, VkIndexType indexType);
static void __GTP_HCB_PFN_vkGetRenderingAreaGranularityKHR(VkDevice device, const VkRenderingAreaInfo *pRenderingAreaInfo, VkExtent2D *pGranularity);
static void __GTP_HCB_PFN_vkGetDeviceImageSubresourceLayoutKHR(VkDevice device, const VkDeviceImageSubresourceInfo *pInfo, VkSubresourceLayout2 *pLayout);
static void __GTP_HCB_PFN_vkGetImageSubresourceLayout2KHR(VkDevice device, VkImage image, const VkImageSubresource2 *pSubresource, VkSubresourceLayout2 *pLayout);
static VkResult __GTP_HCB_PFN_vkWaitForPresent2KHR(VkDevice device, VkSwapchainKHR swapchain, const VkPresentWait2InfoKHR *pPresentWait2Info);
static VkResult __GTP_HCB_PFN_vkCreatePipelineBinariesKHR(VkDevice device, const VkPipelineBinaryCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkPipelineBinaryHandlesInfoKHR *pBinaries);
static void __GTP_HCB_PFN_vkDestroyPipelineBinaryKHR(VkDevice device, VkPipelineBinaryKHR pipelineBinary, const VkAllocationCallbacks *pAllocator);
static VkResult __GTP_HCB_PFN_vkGetPipelineKeyKHR(VkDevice device, const VkPipelineCreateInfoKHR *pPipelineCreateInfo, VkPipelineBinaryKeyKHR *pPipelineKey);
static VkResult __GTP_HCB_PFN_vkGetPipelineBinaryDataKHR(VkDevice device, const VkPipelineBinaryDataInfoKHR *pInfo, VkPipelineBinaryKeyKHR *pPipelineBinaryKey, size_t *pPipelineBinaryDataSize, void *pPipelineBinaryData);
static VkResult __GTP_HCB_PFN_vkReleaseCapturedPipelineDataKHR(VkDevice device, const VkReleaseCapturedPipelineDataInfoKHR *pInfo, const VkAllocationCallbacks *pAllocator);
static VkResult __GTP_HCB_PFN_vkReleaseSwapchainImagesKHR(VkDevice device, const VkReleaseSwapchainImagesInfoKHR *pReleaseInfo);
static VkResult __GTP_HCB_PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount, VkCooperativeMatrixPropertiesKHR *pProperties);
static void __GTP_HCB_PFN_vkCmdSetLineStippleKHR(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern);
static VkResult __GTP_HCB_PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsKHR(VkPhysicalDevice physicalDevice, uint32_t *pTimeDomainCount, VkTimeDomainKHR *pTimeDomains);
static VkResult __GTP_HCB_PFN_vkGetCalibratedTimestampsKHR(VkDevice device, uint32_t timestampCount, const VkCalibratedTimestampInfoKHR *pTimestampInfos, uint64_t *pTimestamps, uint64_t *pMaxDeviation);
static void __GTP_HCB_PFN_vkCmdBindDescriptorSets2KHR(VkCommandBuffer commandBuffer, const VkBindDescriptorSetsInfo *pBindDescriptorSetsInfo);
static void __GTP_HCB_PFN_vkCmdPushConstants2KHR(VkCommandBuffer commandBuffer, const VkPushConstantsInfo *pPushConstantsInfo);
static void __GTP_HCB_PFN_vkCmdPushDescriptorSet2KHR(VkCommandBuffer commandBuffer, const VkPushDescriptorSetInfo *pPushDescriptorSetInfo);
static void __GTP_HCB_PFN_vkCmdPushDescriptorSetWithTemplate2KHR(VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfo *pPushDescriptorSetWithTemplateInfo);
static void __GTP_HCB_PFN_vkCmdSetDescriptorBufferOffsets2EXT(VkCommandBuffer commandBuffer, const VkSetDescriptorBufferOffsetsInfoEXT *pSetDescriptorBufferOffsetsInfo);
static void __GTP_HCB_PFN_vkCmdBindDescriptorBufferEmbeddedSamplers2EXT(VkCommandBuffer commandBuffer, const VkBindDescriptorBufferEmbeddedSamplersInfoEXT *pBindDescriptorBufferEmbeddedSamplersInfo);
static VkResult __GTP_HCB_PFN_vkCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugReportCallbackEXT *pCallback);
static void __GTP_HCB_PFN_vkDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks *pAllocator);
static void __GTP_HCB_PFN_vkDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char *pLayerPrefix, const char *pMessage);
static VkResult __GTP_HCB_PFN_vkDebugMarkerSetObjectTagEXT(VkDevice device, const VkDebugMarkerObjectTagInfoEXT *pTagInfo);
static VkResult __GTP_HCB_PFN_vkDebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT *pNameInfo);
static void __GTP_HCB_PFN_vkCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT *pMarkerInfo);
static void __GTP_HCB_PFN_vkCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer);
static void __GTP_HCB_PFN_vkCmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT *pMarkerInfo);
static void __GTP_HCB_PFN_vkCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer *pBuffers, const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes);
static void __GTP_HCB_PFN_vkCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer, uint32_t counterBufferCount, const VkBuffer *pCounterBuffers, const VkDeviceSize *pCounterBufferOffsets);
static void __GTP_HCB_PFN_vkCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer, uint32_t counterBufferCount, const VkBuffer *pCounterBuffers, const VkDeviceSize *pCounterBufferOffsets);
static void __GTP_HCB_PFN_vkCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags, uint32_t index);
static void __GTP_HCB_PFN_vkCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, uint32_t index);
static void __GTP_HCB_PFN_vkCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance, VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset, uint32_t vertexStride);
static VkResult __GTP_HCB_PFN_vkCreateCuModuleNVX(VkDevice device, const VkCuModuleCreateInfoNVX *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkCuModuleNVX *pModule);
static VkResult __GTP_HCB_PFN_vkCreateCuFunctionNVX(VkDevice device, const VkCuFunctionCreateInfoNVX *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkCuFunctionNVX *pFunction);
static void __GTP_HCB_PFN_vkDestroyCuModuleNVX(VkDevice device, VkCuModuleNVX module, const VkAllocationCallbacks *pAllocator);
static void __GTP_HCB_PFN_vkDestroyCuFunctionNVX(VkDevice device, VkCuFunctionNVX function, const VkAllocationCallbacks *pAllocator);
static void __GTP_HCB_PFN_vkCmdCuLaunchKernelNVX(VkCommandBuffer commandBuffer, const VkCuLaunchInfoNVX *pLaunchInfo);
static uint32_t __GTP_HCB_PFN_vkGetImageViewHandleNVX(VkDevice device, const VkImageViewHandleInfoNVX *pInfo);
static uint64_t __GTP_HCB_PFN_vkGetImageViewHandle64NVX(VkDevice device, const VkImageViewHandleInfoNVX *pInfo);
static VkResult __GTP_HCB_PFN_vkGetImageViewAddressNVX(VkDevice device, VkImageView imageView, VkImageViewAddressPropertiesNVX *pProperties);
static void __GTP_HCB_PFN_vkCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);
static void __GTP_HCB_PFN_vkCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);
static VkResult __GTP_HCB_PFN_vkGetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage, VkShaderInfoTypeAMD infoType, size_t *pInfoSize, void *pInfo);
static VkResult __GTP_HCB_PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType, VkExternalImageFormatPropertiesNV *pExternalImageFormatProperties);
static void __GTP_HCB_PFN_vkCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfoEXT *pConditionalRenderingBegin);
static void __GTP_HCB_PFN_vkCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer);
static void __GTP_HCB_PFN_vkCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewportWScalingNV *pViewportWScalings);
static VkResult __GTP_HCB_PFN_vkReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display);
static VkResult __GTP_HCB_PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilities2EXT *pSurfaceCapabilities);
static VkResult __GTP_HCB_PFN_vkDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display, const VkDisplayPowerInfoEXT *pDisplayPowerInfo);
static VkResult __GTP_HCB_PFN_vkRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT *pDeviceEventInfo, const VkAllocationCallbacks *pAllocator, VkFence *pFence);
static VkResult __GTP_HCB_PFN_vkRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display, const VkDisplayEventInfoEXT *pDisplayEventInfo, const VkAllocationCallbacks *pAllocator, VkFence *pFence);
static VkResult __GTP_HCB_PFN_vkGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter, uint64_t *pCounterValue);
static VkResult __GTP_HCB_PFN_vkGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain, VkRefreshCycleDurationGOOGLE *pDisplayTimingProperties);
static VkResult __GTP_HCB_PFN_vkGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain, uint32_t *pPresentationTimingCount, VkPastPresentationTimingGOOGLE *pPresentationTimings);
static void __GTP_HCB_PFN_vkCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle, uint32_t discardRectangleCount, const VkRect2D *pDiscardRectangles);
static void __GTP_HCB_PFN_vkCmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 discardRectangleEnable);
static void __GTP_HCB_PFN_vkCmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer, VkDiscardRectangleModeEXT discardRectangleMode);
static void __GTP_HCB_PFN_vkSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR *pSwapchains, const VkHdrMetadataEXT *pMetadata);
static VkResult __GTP_HCB_PFN_vkSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT *pNameInfo);
static VkResult __GTP_HCB_PFN_vkSetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT *pTagInfo);
static void __GTP_HCB_PFN_vkQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT *pLabelInfo);
static void __GTP_HCB_PFN_vkQueueEndDebugUtilsLabelEXT(VkQueue queue);
static void __GTP_HCB_PFN_vkQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT *pLabelInfo);
static void __GTP_HCB_PFN_vkCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT *pLabelInfo);
static void __GTP_HCB_PFN_vkCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer);
static void __GTP_HCB_PFN_vkCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT *pLabelInfo);
static VkResult __GTP_HCB_PFN_vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pMessenger);
static void __GTP_HCB_PFN_vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks *pAllocator);
static void __GTP_HCB_PFN_vkSubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData);
static void __GTP_HCB_PFN_vkCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer, const VkSampleLocationsInfoEXT *pSampleLocationsInfo);
static void __GTP_HCB_PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice physicalDevice, VkSampleCountFlagBits samples, VkMultisamplePropertiesEXT *pMultisampleProperties);
static VkResult __GTP_HCB_PFN_vkGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image, VkImageDrmFormatModifierPropertiesEXT *pProperties);
static VkResult __GTP_HCB_PFN_vkCreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkValidationCacheEXT *pValidationCache);
static void __GTP_HCB_PFN_vkDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache, const VkAllocationCallbacks *pAllocator);
static VkResult __GTP_HCB_PFN_vkMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount, const VkValidationCacheEXT *pSrcCaches);
static VkResult __GTP_HCB_PFN_vkGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t *pDataSize, void *pData);
static void __GTP_HCB_PFN_vkCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout);
static void __GTP_HCB_PFN_vkCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkShadingRatePaletteNV *pShadingRatePalettes);
static void __GTP_HCB_PFN_vkCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType, uint32_t customSampleOrderCount, const VkCoarseSampleOrderCustomNV *pCustomSampleOrders);
static VkResult __GTP_HCB_PFN_vkCreateAccelerationStructureNV(VkDevice device, const VkAccelerationStructureCreateInfoNV *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkAccelerationStructureNV *pAccelerationStructure);
static void __GTP_HCB_PFN_vkDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure, const VkAllocationCallbacks *pAllocator);
static void __GTP_HCB_PFN_vkGetAccelerationStructureMemoryRequirementsNV(VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoNV *pInfo, VkMemoryRequirements2KHR *pMemoryRequirements);
static VkResult __GTP_HCB_PFN_vkBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount, const VkBindAccelerationStructureMemoryInfoNV *pBindInfos);
static void __GTP_HCB_PFN_vkCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV *pInfo, VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update, VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkBuffer scratch, VkDeviceSize scratchOffset);
static void __GTP_HCB_PFN_vkCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkCopyAccelerationStructureModeKHR mode);
static void __GTP_HCB_PFN_vkCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer, VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer, VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride, VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset, VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer, VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride, uint32_t width, uint32_t height, uint32_t depth);
static VkResult __GTP_HCB_PFN_vkCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoNV *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines);
static VkResult __GTP_HCB_PFN_vkGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void *pData);
static VkResult __GTP_HCB_PFN_vkGetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void *pData);
static VkResult __GTP_HCB_PFN_vkGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure, size_t dataSize, void *pData);
static void __GTP_HCB_PFN_vkCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureNV *pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery);
static VkResult __GTP_HCB_PFN_vkCompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader);
static VkResult __GTP_HCB_PFN_vkGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, const void *pHostPointer, VkMemoryHostPointerPropertiesEXT *pMemoryHostPointerProperties);
static void __GTP_HCB_PFN_vkCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker);
static void __GTP_HCB_PFN_vkCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker);
static VkResult __GTP_HCB_PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT(VkPhysicalDevice physicalDevice, uint32_t *pTimeDomainCount, VkTimeDomainKHR *pTimeDomains);
static VkResult __GTP_HCB_PFN_vkGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount, const VkCalibratedTimestampInfoKHR *pTimestampInfos, uint64_t *pTimestamps, uint64_t *pMaxDeviation);
static void __GTP_HCB_PFN_vkCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask);
static void __GTP_HCB_PFN_vkCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
static void __GTP_HCB_PFN_vkCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);
static void __GTP_HCB_PFN_vkCmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor, uint32_t exclusiveScissorCount, const VkBool32 *pExclusiveScissorEnables);
static void __GTP_HCB_PFN_vkCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor, uint32_t exclusiveScissorCount, const VkRect2D *pExclusiveScissors);
static void __GTP_HCB_PFN_vkCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void *pCheckpointMarker);
static void __GTP_HCB_PFN_vkGetQueueCheckpointDataNV(VkQueue queue, uint32_t *pCheckpointDataCount, VkCheckpointDataNV *pCheckpointData);
static void __GTP_HCB_PFN_vkGetQueueCheckpointData2NV(VkQueue queue, uint32_t *pCheckpointDataCount, VkCheckpointData2NV *pCheckpointData);
static VkResult __GTP_HCB_PFN_vkInitializePerformanceApiINTEL(VkDevice device, const VkInitializePerformanceApiInfoINTEL *pInitializeInfo);
static void __GTP_HCB_PFN_vkUninitializePerformanceApiINTEL(VkDevice device);
static VkResult __GTP_HCB_PFN_vkCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer, const VkPerformanceMarkerInfoINTEL *pMarkerInfo);
static VkResult __GTP_HCB_PFN_vkCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer, const VkPerformanceStreamMarkerInfoINTEL *pMarkerInfo);
static VkResult __GTP_HCB_PFN_vkCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer, const VkPerformanceOverrideInfoINTEL *pOverrideInfo);
static VkResult __GTP_HCB_PFN_vkAcquirePerformanceConfigurationINTEL(VkDevice device, const VkPerformanceConfigurationAcquireInfoINTEL *pAcquireInfo, VkPerformanceConfigurationINTEL *pConfiguration);
static VkResult __GTP_HCB_PFN_vkReleasePerformanceConfigurationINTEL(VkDevice device, VkPerformanceConfigurationINTEL configuration);
static VkResult __GTP_HCB_PFN_vkQueueSetPerformanceConfigurationINTEL(VkQueue queue, VkPerformanceConfigurationINTEL configuration);
static VkResult __GTP_HCB_PFN_vkGetPerformanceParameterINTEL(VkDevice device, VkPerformanceParameterTypeINTEL parameter, VkPerformanceValueINTEL *pValue);
static void __GTP_HCB_PFN_vkSetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapChain, VkBool32 localDimmingEnable);
static VkDeviceAddress __GTP_HCB_PFN_vkGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo *pInfo);
static VkResult __GTP_HCB_PFN_vkGetPhysicalDeviceToolPropertiesEXT(VkPhysicalDevice physicalDevice, uint32_t *pToolCount, VkPhysicalDeviceToolProperties *pToolProperties);
static VkResult __GTP_HCB_PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount, VkCooperativeMatrixPropertiesNV *pProperties);
static VkResult __GTP_HCB_PFN_vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(VkPhysicalDevice physicalDevice, uint32_t *pCombinationCount, VkFramebufferMixedSamplesCombinationNV *pCombinations);
static VkResult __GTP_HCB_PFN_vkCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface);
static void __GTP_HCB_PFN_vkCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern);
static void __GTP_HCB_PFN_vkResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);
static void __GTP_HCB_PFN_vkCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode);
static void __GTP_HCB_PFN_vkCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace);
static void __GTP_HCB_PFN_vkCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology);
static void __GTP_HCB_PFN_vkCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport *pViewports);
static void __GTP_HCB_PFN_vkCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D *pScissors);
static void __GTP_HCB_PFN_vkCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer *pBuffers, const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes, const VkDeviceSize *pStrides);
static void __GTP_HCB_PFN_vkCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable);
static void __GTP_HCB_PFN_vkCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable);
static void __GTP_HCB_PFN_vkCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp);
static void __GTP_HCB_PFN_vkCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable);
static void __GTP_HCB_PFN_vkCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable);
static void __GTP_HCB_PFN_vkCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp);
static VkResult __GTP_HCB_PFN_vkCopyMemoryToImageEXT(VkDevice device, const VkCopyMemoryToImageInfo *pCopyMemoryToImageInfo);
static VkResult __GTP_HCB_PFN_vkCopyImageToMemoryEXT(VkDevice device, const VkCopyImageToMemoryInfo *pCopyImageToMemoryInfo);
static VkResult __GTP_HCB_PFN_vkCopyImageToImageEXT(VkDevice device, const VkCopyImageToImageInfo *pCopyImageToImageInfo);
static VkResult __GTP_HCB_PFN_vkTransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount, const VkHostImageLayoutTransitionInfo *pTransitions);
static void __GTP_HCB_PFN_vkGetImageSubresourceLayout2EXT(VkDevice device, VkImage image, const VkImageSubresource2 *pSubresource, VkSubresourceLayout2 *pLayout);
static VkResult __GTP_HCB_PFN_vkReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoKHR *pReleaseInfo);
static void __GTP_HCB_PFN_vkGetGeneratedCommandsMemoryRequirementsNV(VkDevice device, const VkGeneratedCommandsMemoryRequirementsInfoNV *pInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __GTP_HCB_PFN_vkCmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer, const VkGeneratedCommandsInfoNV *pGeneratedCommandsInfo);
static void __GTP_HCB_PFN_vkCmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed, const VkGeneratedCommandsInfoNV *pGeneratedCommandsInfo);
static void __GTP_HCB_PFN_vkCmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline, uint32_t groupIndex);
static VkResult __GTP_HCB_PFN_vkCreateIndirectCommandsLayoutNV(VkDevice device, const VkIndirectCommandsLayoutCreateInfoNV *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkIndirectCommandsLayoutNV *pIndirectCommandsLayout);
static void __GTP_HCB_PFN_vkDestroyIndirectCommandsLayoutNV(VkDevice device, VkIndirectCommandsLayoutNV indirectCommandsLayout, const VkAllocationCallbacks *pAllocator);
static void __GTP_HCB_PFN_vkCmdSetDepthBias2EXT(VkCommandBuffer commandBuffer, const VkDepthBiasInfoEXT *pDepthBiasInfo);
static VkResult __GTP_HCB_PFN_vkAcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, VkDisplayKHR display);
static VkResult __GTP_HCB_PFN_vkGetDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, uint32_t connectorId, VkDisplayKHR *display);
static VkResult __GTP_HCB_PFN_vkCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkPrivateDataSlot *pPrivateDataSlot);
static void __GTP_HCB_PFN_vkDestroyPrivateDataSlotEXT(VkDevice device, VkPrivateDataSlot privateDataSlot, const VkAllocationCallbacks *pAllocator);
static VkResult __GTP_HCB_PFN_vkSetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlot privateDataSlot, uint64_t data);
static void __GTP_HCB_PFN_vkGetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlot privateDataSlot, uint64_t *pData);
static void __GTP_HCB_PFN_vkCmdDispatchTileQCOM(VkCommandBuffer commandBuffer, const VkDispatchTileInfoQCOM *pDispatchTileInfo);
static void __GTP_HCB_PFN_vkCmdBeginPerTileExecutionQCOM(VkCommandBuffer commandBuffer, const VkPerTileBeginInfoQCOM *pPerTileBeginInfo);
static void __GTP_HCB_PFN_vkCmdEndPerTileExecutionQCOM(VkCommandBuffer commandBuffer, const VkPerTileEndInfoQCOM *pPerTileEndInfo);
static void __GTP_HCB_PFN_vkGetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout, VkDeviceSize *pLayoutSizeInBytes);
static void __GTP_HCB_PFN_vkGetDescriptorSetLayoutBindingOffsetEXT(VkDevice device, VkDescriptorSetLayout layout, uint32_t binding, VkDeviceSize *pOffset);
static void __GTP_HCB_PFN_vkGetDescriptorEXT(VkDevice device, const VkDescriptorGetInfoEXT *pDescriptorInfo, size_t dataSize, void *pDescriptor);
static void __GTP_HCB_PFN_vkCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount, const VkDescriptorBufferBindingInfoEXT *pBindingInfos);
static void __GTP_HCB_PFN_vkCmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount, const uint32_t *pBufferIndices, const VkDeviceSize *pOffsets);
static void __GTP_HCB_PFN_vkCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t set);
static VkResult __GTP_HCB_PFN_vkGetBufferOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkBufferCaptureDescriptorDataInfoEXT *pInfo, void *pData);
static VkResult __GTP_HCB_PFN_vkGetImageOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkImageCaptureDescriptorDataInfoEXT *pInfo, void *pData);
static VkResult __GTP_HCB_PFN_vkGetImageViewOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkImageViewCaptureDescriptorDataInfoEXT *pInfo, void *pData);
static VkResult __GTP_HCB_PFN_vkGetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkSamplerCaptureDescriptorDataInfoEXT *pInfo, void *pData);
static VkResult __GTP_HCB_PFN_vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkAccelerationStructureCaptureDescriptorDataInfoEXT *pInfo, void *pData);
static void __GTP_HCB_PFN_vkCmdSetFragmentShadingRateEnumNV(VkCommandBuffer commandBuffer, VkFragmentShadingRateNV shadingRate, const VkFragmentShadingRateCombinerOpKHR combinerOps[2]);
static VkResult __GTP_HCB_PFN_vkGetDeviceFaultInfoEXT(VkDevice device, VkDeviceFaultCountsEXT *pFaultCounts, VkDeviceFaultInfoEXT *pFaultInfo);
static void __GTP_HCB_PFN_vkCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount, const VkVertexInputBindingDescription2EXT *pVertexBindingDescriptions, uint32_t vertexAttributeDescriptionCount, const VkVertexInputAttributeDescription2EXT *pVertexAttributeDescriptions);
static VkResult __GTP_HCB_PFN_vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(VkDevice device, VkRenderPass renderpass, VkExtent2D *pMaxWorkgroupSize);
static void __GTP_HCB_PFN_vkCmdSubpassShadingHUAWEI(VkCommandBuffer commandBuffer);
static void __GTP_HCB_PFN_vkCmdBindInvocationMaskHUAWEI(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout);
static VkResult __GTP_HCB_PFN_vkGetMemoryRemoteAddressNV(VkDevice device, const VkMemoryGetRemoteAddressInfoNV *pMemoryGetRemoteAddressInfo, VkRemoteAddressNV *pAddress);
static VkResult __GTP_HCB_PFN_vkGetPipelinePropertiesEXT(VkDevice device, const VkPipelineInfoEXT *pPipelineInfo, VkBaseOutStructure *pPipelineProperties);
static void __GTP_HCB_PFN_vkCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints);
static void __GTP_HCB_PFN_vkCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable);
static void __GTP_HCB_PFN_vkCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable);
static void __GTP_HCB_PFN_vkCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp);
static void __GTP_HCB_PFN_vkCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable);
static void __GTP_HCB_PFN_vkCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkBool32 *pColorWriteEnables);
static void __GTP_HCB_PFN_vkCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT *pVertexInfo, uint32_t instanceCount, uint32_t firstInstance, uint32_t stride);
static void __GTP_HCB_PFN_vkCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawIndexedInfoEXT *pIndexInfo, uint32_t instanceCount, uint32_t firstInstance, uint32_t stride, const int32_t *pVertexOffset);
static VkResult __GTP_HCB_PFN_vkCreateMicromapEXT(VkDevice device, const VkMicromapCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkMicromapEXT *pMicromap);
static void __GTP_HCB_PFN_vkDestroyMicromapEXT(VkDevice device, VkMicromapEXT micromap, const VkAllocationCallbacks *pAllocator);
static void __GTP_HCB_PFN_vkCmdBuildMicromapsEXT(VkCommandBuffer commandBuffer, uint32_t infoCount, const VkMicromapBuildInfoEXT *pInfos);
static VkResult __GTP_HCB_PFN_vkBuildMicromapsEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount, const VkMicromapBuildInfoEXT *pInfos);
static VkResult __GTP_HCB_PFN_vkCopyMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyMicromapInfoEXT *pInfo);
static VkResult __GTP_HCB_PFN_vkCopyMicromapToMemoryEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyMicromapToMemoryInfoEXT *pInfo);
static VkResult __GTP_HCB_PFN_vkCopyMemoryToMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyMemoryToMicromapInfoEXT *pInfo);
static VkResult __GTP_HCB_PFN_vkWriteMicromapsPropertiesEXT(VkDevice device, uint32_t micromapCount, const VkMicromapEXT *pMicromaps, VkQueryType queryType, size_t dataSize, void *pData, size_t stride);
static void __GTP_HCB_PFN_vkCmdCopyMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapInfoEXT *pInfo);
static void __GTP_HCB_PFN_vkCmdCopyMicromapToMemoryEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapToMemoryInfoEXT *pInfo);
static void __GTP_HCB_PFN_vkCmdCopyMemoryToMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMemoryToMicromapInfoEXT *pInfo);
static void __GTP_HCB_PFN_vkCmdWriteMicromapsPropertiesEXT(VkCommandBuffer commandBuffer, uint32_t micromapCount, const VkMicromapEXT *pMicromaps, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery);
static void __GTP_HCB_PFN_vkGetDeviceMicromapCompatibilityEXT(VkDevice device, const VkMicromapVersionInfoEXT *pVersionInfo, VkAccelerationStructureCompatibilityKHR *pCompatibility);
static void __GTP_HCB_PFN_vkGetMicromapBuildSizesEXT(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType, const VkMicromapBuildInfoEXT *pBuildInfo, VkMicromapBuildSizesInfoEXT *pSizeInfo);
static void __GTP_HCB_PFN_vkCmdDrawClusterHUAWEI(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
static void __GTP_HCB_PFN_vkCmdDrawClusterIndirectHUAWEI(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset);
static void __GTP_HCB_PFN_vkSetDeviceMemoryPriorityEXT(VkDevice device, VkDeviceMemory memory, float priority);
static void __GTP_HCB_PFN_vkGetDescriptorSetLayoutHostMappingInfoVALVE(VkDevice device, const VkDescriptorSetBindingReferenceVALVE *pBindingReference, VkDescriptorSetLayoutHostMappingInfoVALVE *pHostMapping);
static void __GTP_HCB_PFN_vkGetDescriptorSetHostMappingVALVE(VkDevice device, VkDescriptorSet descriptorSet, void **ppData);
static void __GTP_HCB_PFN_vkCmdCopyMemoryIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress, uint32_t copyCount, uint32_t stride);
static void __GTP_HCB_PFN_vkCmdCopyMemoryToImageIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress, uint32_t copyCount, uint32_t stride, VkImage dstImage, VkImageLayout dstImageLayout, const VkImageSubresourceLayers *pImageSubresources);
static void __GTP_HCB_PFN_vkCmdDecompressMemoryNV(VkCommandBuffer commandBuffer, uint32_t decompressRegionCount, const VkDecompressMemoryRegionNV *pDecompressMemoryRegions);
static void __GTP_HCB_PFN_vkCmdDecompressMemoryIndirectCountNV(VkCommandBuffer commandBuffer, VkDeviceAddress indirectCommandsAddress, VkDeviceAddress indirectCommandsCountAddress, uint32_t stride);
static void __GTP_HCB_PFN_vkGetPipelineIndirectMemoryRequirementsNV(VkDevice device, const VkComputePipelineCreateInfo *pCreateInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __GTP_HCB_PFN_vkCmdUpdatePipelineIndirectBufferNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);
static VkDeviceAddress __GTP_HCB_PFN_vkGetPipelineIndirectDeviceAddressNV(VkDevice device, const VkPipelineIndirectDeviceAddressInfoNV *pInfo);
static void __GTP_HCB_PFN_vkCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable);
static void __GTP_HCB_PFN_vkCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode);
static void __GTP_HCB_PFN_vkCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits rasterizationSamples);
static void __GTP_HCB_PFN_vkCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples, const VkSampleMask *pSampleMask);
static void __GTP_HCB_PFN_vkCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToCoverageEnable);
static void __GTP_HCB_PFN_vkCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable);
static void __GTP_HCB_PFN_vkCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable);
static void __GTP_HCB_PFN_vkCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount, const VkBool32 *pColorBlendEnables);
static void __GTP_HCB_PFN_vkCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount, const VkColorBlendEquationEXT *pColorBlendEquations);
static void __GTP_HCB_PFN_vkCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount, const VkColorComponentFlags *pColorWriteMasks);
static void __GTP_HCB_PFN_vkCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer, VkTessellationDomainOrigin domainOrigin);
static void __GTP_HCB_PFN_vkCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream);
static void __GTP_HCB_PFN_vkCmdSetConservativeRasterizationModeEXT(VkCommandBuffer commandBuffer, VkConservativeRasterizationModeEXT conservativeRasterizationMode);
static void __GTP_HCB_PFN_vkCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer, float extraPrimitiveOverestimationSize);
static void __GTP_HCB_PFN_vkCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable);
static void __GTP_HCB_PFN_vkCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer, VkBool32 sampleLocationsEnable);
static void __GTP_HCB_PFN_vkCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount, const VkColorBlendAdvancedEXT *pColorBlendAdvanced);
static void __GTP_HCB_PFN_vkCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer, VkProvokingVertexModeEXT provokingVertexMode);
static void __GTP_HCB_PFN_vkCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer, VkLineRasterizationModeEXT lineRasterizationMode);
static void __GTP_HCB_PFN_vkCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable);
static void __GTP_HCB_PFN_vkCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne);
static void __GTP_HCB_PFN_vkCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer, VkBool32 viewportWScalingEnable);
static void __GTP_HCB_PFN_vkCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewportSwizzleNV *pViewportSwizzles);
static void __GTP_HCB_PFN_vkCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageToColorEnable);
static void __GTP_HCB_PFN_vkCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer, uint32_t coverageToColorLocation);
static void __GTP_HCB_PFN_vkCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer, VkCoverageModulationModeNV coverageModulationMode);
static void __GTP_HCB_PFN_vkCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageModulationTableEnable);
static void __GTP_HCB_PFN_vkCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer, uint32_t coverageModulationTableCount, const float *pCoverageModulationTable);
static void __GTP_HCB_PFN_vkCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer, VkBool32 shadingRateImageEnable);
static void __GTP_HCB_PFN_vkCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer, VkBool32 representativeFragmentTestEnable);
static void __GTP_HCB_PFN_vkCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer, VkCoverageReductionModeNV coverageReductionMode);
static void __GTP_HCB_PFN_vkGetShaderModuleIdentifierEXT(VkDevice device, VkShaderModule shaderModule, VkShaderModuleIdentifierEXT *pIdentifier);
static void __GTP_HCB_PFN_vkGetShaderModuleCreateInfoIdentifierEXT(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo, VkShaderModuleIdentifierEXT *pIdentifier);
static VkResult __GTP_HCB_PFN_vkGetPhysicalDeviceOpticalFlowImageFormatsNV(VkPhysicalDevice physicalDevice, const VkOpticalFlowImageFormatInfoNV *pOpticalFlowImageFormatInfo, uint32_t *pFormatCount, VkOpticalFlowImageFormatPropertiesNV *pImageFormatProperties);
static VkResult __GTP_HCB_PFN_vkCreateOpticalFlowSessionNV(VkDevice device, const VkOpticalFlowSessionCreateInfoNV *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkOpticalFlowSessionNV *pSession);
static void __GTP_HCB_PFN_vkDestroyOpticalFlowSessionNV(VkDevice device, VkOpticalFlowSessionNV session, const VkAllocationCallbacks *pAllocator);
static VkResult __GTP_HCB_PFN_vkBindOpticalFlowSessionImageNV(VkDevice device, VkOpticalFlowSessionNV session, VkOpticalFlowSessionBindingPointNV bindingPoint, VkImageView view, VkImageLayout layout);
static void __GTP_HCB_PFN_vkCmdOpticalFlowExecuteNV(VkCommandBuffer commandBuffer, VkOpticalFlowSessionNV session, const VkOpticalFlowExecuteInfoNV *pExecuteInfo);
static void __GTP_HCB_PFN_vkAntiLagUpdateAMD(VkDevice device, const VkAntiLagDataAMD *pData);
static VkResult __GTP_HCB_PFN_vkCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders);
static void __GTP_HCB_PFN_vkDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks *pAllocator);
static VkResult __GTP_HCB_PFN_vkGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t *pDataSize, void *pData);
static void __GTP_HCB_PFN_vkCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount, const VkShaderStageFlagBits *pStages, const VkShaderEXT *pShaders);
static void __GTP_HCB_PFN_vkCmdSetDepthClampRangeEXT(VkCommandBuffer commandBuffer, VkDepthClampModeEXT depthClampMode, const VkDepthClampRangeEXT *pDepthClampRange);
static VkResult __GTP_HCB_PFN_vkGetFramebufferTilePropertiesQCOM(VkDevice device, VkFramebuffer framebuffer, uint32_t *pPropertiesCount, VkTilePropertiesQCOM *pProperties);
static VkResult __GTP_HCB_PFN_vkGetDynamicRenderingTilePropertiesQCOM(VkDevice device, const VkRenderingInfo *pRenderingInfo, VkTilePropertiesQCOM *pProperties);
static VkResult __GTP_HCB_PFN_vkGetPhysicalDeviceCooperativeVectorPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount, VkCooperativeVectorPropertiesNV *pProperties);
static VkResult __GTP_HCB_PFN_vkConvertCooperativeVectorMatrixNV(VkDevice device, const VkConvertCooperativeVectorMatrixInfoNV *pInfo);
static void __GTP_HCB_PFN_vkCmdConvertCooperativeVectorMatrixNV(VkCommandBuffer commandBuffer, uint32_t infoCount, const VkConvertCooperativeVectorMatrixInfoNV *pInfos);
static VkResult __GTP_HCB_PFN_vkSetLatencySleepModeNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepModeInfoNV *pSleepModeInfo);
static VkResult __GTP_HCB_PFN_vkLatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV *pSleepInfo);
static void __GTP_HCB_PFN_vkSetLatencyMarkerNV(VkDevice device, VkSwapchainKHR swapchain, const VkSetLatencyMarkerInfoNV *pLatencyMarkerInfo);
static void __GTP_HCB_PFN_vkGetLatencyTimingsNV(VkDevice device, VkSwapchainKHR swapchain, VkGetLatencyMarkerInfoNV *pLatencyMarkerInfo);
static void __GTP_HCB_PFN_vkQueueNotifyOutOfBandNV(VkQueue queue, const VkOutOfBandQueueTypeInfoNV *pQueueTypeInfo);
static void __GTP_HCB_PFN_vkCmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer commandBuffer, VkImageAspectFlags aspectMask);
static void __GTP_HCB_PFN_vkCmdBindTileMemoryQCOM(VkCommandBuffer commandBuffer, const VkTileMemoryBindInfoQCOM *pTileMemoryBindInfo);
static VkResult __GTP_HCB_PFN_vkCreateExternalComputeQueueNV(VkDevice device, const VkExternalComputeQueueCreateInfoNV *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkExternalComputeQueueNV *pExternalQueue);
static void __GTP_HCB_PFN_vkDestroyExternalComputeQueueNV(VkDevice device, VkExternalComputeQueueNV externalQueue, const VkAllocationCallbacks *pAllocator);
static void __GTP_HCB_PFN_vkGetExternalComputeQueueDataNV(VkExternalComputeQueueNV externalQueue, VkExternalComputeQueueDataParamsNV *params, void *pData);
static void __GTP_HCB_PFN_vkGetClusterAccelerationStructureBuildSizesNV(VkDevice device, const VkClusterAccelerationStructureInputInfoNV *pInfo, VkAccelerationStructureBuildSizesInfoKHR *pSizeInfo);
static void __GTP_HCB_PFN_vkCmdBuildClusterAccelerationStructureIndirectNV(VkCommandBuffer commandBuffer, const VkClusterAccelerationStructureCommandsInfoNV *pCommandInfos);
static void __GTP_HCB_PFN_vkGetPartitionedAccelerationStructuresBuildSizesNV(VkDevice device, const VkPartitionedAccelerationStructureInstancesInputNV *pInfo, VkAccelerationStructureBuildSizesInfoKHR *pSizeInfo);
static void __GTP_HCB_PFN_vkCmdBuildPartitionedAccelerationStructuresNV(VkCommandBuffer commandBuffer, const VkBuildPartitionedAccelerationStructureInfoNV *pBuildInfo);
static void __GTP_HCB_PFN_vkGetGeneratedCommandsMemoryRequirementsEXT(VkDevice device, const VkGeneratedCommandsMemoryRequirementsInfoEXT *pInfo, VkMemoryRequirements2 *pMemoryRequirements);
static void __GTP_HCB_PFN_vkCmdPreprocessGeneratedCommandsEXT(VkCommandBuffer commandBuffer, const VkGeneratedCommandsInfoEXT *pGeneratedCommandsInfo, VkCommandBuffer stateCommandBuffer);
static void __GTP_HCB_PFN_vkCmdExecuteGeneratedCommandsEXT(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed, const VkGeneratedCommandsInfoEXT *pGeneratedCommandsInfo);
static VkResult __GTP_HCB_PFN_vkCreateIndirectCommandsLayoutEXT(VkDevice device, const VkIndirectCommandsLayoutCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkIndirectCommandsLayoutEXT *pIndirectCommandsLayout);
static void __GTP_HCB_PFN_vkDestroyIndirectCommandsLayoutEXT(VkDevice device, VkIndirectCommandsLayoutEXT indirectCommandsLayout, const VkAllocationCallbacks *pAllocator);
static VkResult __GTP_HCB_PFN_vkCreateIndirectExecutionSetEXT(VkDevice device, const VkIndirectExecutionSetCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkIndirectExecutionSetEXT *pIndirectExecutionSet);
static void __GTP_HCB_PFN_vkDestroyIndirectExecutionSetEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet, const VkAllocationCallbacks *pAllocator);
static void __GTP_HCB_PFN_vkUpdateIndirectExecutionSetPipelineEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet, uint32_t executionSetWriteCount, const VkWriteIndirectExecutionSetPipelineEXT *pExecutionSetWrites);
static void __GTP_HCB_PFN_vkUpdateIndirectExecutionSetShaderEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet, uint32_t executionSetWriteCount, const VkWriteIndirectExecutionSetShaderEXT *pExecutionSetWrites);
static VkResult __GTP_HCB_PFN_vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount, VkCooperativeMatrixFlexibleDimensionsPropertiesNV *pProperties);
static void __GTP_HCB_PFN_vkCmdEndRendering2EXT(VkCommandBuffer commandBuffer, const VkRenderingEndInfoEXT *pRenderingEndInfo);
static VkResult __GTP_HCB_PFN_vkCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkAccelerationStructureKHR *pAccelerationStructure);
static void __GTP_HCB_PFN_vkDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure, const VkAllocationCallbacks *pAllocator);
static void __GTP_HCB_PFN_vkCmdBuildAccelerationStructuresKHR(VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos, const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos);
static void __GTP_HCB_PFN_vkCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos, const VkDeviceAddress *pIndirectDeviceAddresses, const uint32_t *pIndirectStrides, const uint32_t *const *ppMaxPrimitiveCounts);
static VkResult __GTP_HCB_PFN_vkBuildAccelerationStructuresKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos, const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos);
static VkResult __GTP_HCB_PFN_vkCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyAccelerationStructureInfoKHR *pInfo);
static VkResult __GTP_HCB_PFN_vkCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo);
static VkResult __GTP_HCB_PFN_vkCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo);
static VkResult __GTP_HCB_PFN_vkWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR *pAccelerationStructures, VkQueryType queryType, size_t dataSize, void *pData, size_t stride);
static void __GTP_HCB_PFN_vkCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureInfoKHR *pInfo);
static void __GTP_HCB_PFN_vkCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo);
static void __GTP_HCB_PFN_vkCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer, const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo);
static VkDeviceAddress __GTP_HCB_PFN_vkGetAccelerationStructureDeviceAddressKHR(VkDevice device, const VkAccelerationStructureDeviceAddressInfoKHR *pInfo);
static void __GTP_HCB_PFN_vkCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR *pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery);
static void __GTP_HCB_PFN_vkGetDeviceAccelerationStructureCompatibilityKHR(VkDevice device, const VkAccelerationStructureVersionInfoKHR *pVersionInfo, VkAccelerationStructureCompatibilityKHR *pCompatibility);
static void __GTP_HCB_PFN_vkGetAccelerationStructureBuildSizesKHR(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType, const VkAccelerationStructureBuildGeometryInfoKHR *pBuildInfo, const uint32_t *pMaxPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR *pSizeInfo);
static void __GTP_HCB_PFN_vkCmdTraceRaysKHR(VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable, const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable, const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable, const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable, uint32_t width, uint32_t height, uint32_t depth);
static VkResult __GTP_HCB_PFN_vkCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines);
static VkResult __GTP_HCB_PFN_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void *pData);
static void __GTP_HCB_PFN_vkCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable, const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable, const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable, const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable, VkDeviceAddress indirectDeviceAddress);
static VkDeviceSize __GTP_HCB_PFN_vkGetRayTracingShaderGroupStackSizeKHR(VkDevice device, VkPipeline pipeline, uint32_t group, VkShaderGroupShaderKHR groupShader);
static void __GTP_HCB_PFN_vkCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer, uint32_t pipelineStackSize);
static void __GTP_HCB_PFN_vkCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
static void __GTP_HCB_PFN_vkCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
static void __GTP_HCB_PFN_vkCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride);
static VkResult __GTP_HCB_PFN_vkCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface);
static VkBool32 __GTP_HCB_PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, struct wl_display *display);
static VkResult __GTP_HCB_PFN_vkCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface);
static VkBool32 __GTP_HCB_PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, xcb_connection_t *connection, xcb_visualid_t visual_id);
static VkResult __GTP_HCB_PFN_vkCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface);
static VkBool32 __GTP_HCB_PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, Display *dpy, VisualID visualID);

#pragma clang diagnostic pop

#endif

#ifdef __cplusplus
}
#endif

#endif // VULKANAPIS_H
