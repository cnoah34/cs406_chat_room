import { ref, watchEffect } from 'vue'
import axios from 'axios'
import { useApiStore, useWebSocketStore } from '@/store/api'
import { useUserStore } from '@/store/user'


export async function fetchUserData() {
    try {
        const apiStore = useApiStore()
        const response = await axios.get(`${apiStore.rest_url}/users/metadata`, {
            headers: {
                Authorization: `Bearer ${localStorage.getItem('token')}`
            }
        }) 

        const userStore = useUserStore()
        userStore.setUser(response.data)
    }
    catch (error) {
        console.error("Failed to fetch user meta data: ", error)
    }
}

export const isAuthenticated = ref(false)

export const verifyToken = async () => {
    const token = localStorage.getItem('token')

    if (token) {
        try {
            const apiStore = useApiStore()

            const response = await axios.post(`${apiStore.rest_url}/verify-token`, {}, {
                headers: {
                    Authorization: `Bearer ${token}`
                }
            })

            if (response.status == 204) {
                isAuthenticated.value = true
            }
        }
        catch (error) {
            isAuthenticated.value = false
            localStorage.removeItem('token')
        }
    }
    else {
        isAuthenticated.value = false
    }
}

export async function login(token, router) {
    localStorage.setItem('token', token)

    // Clear any stores from previous sessions
    const userStore = useUserStore()
    userStore.clearUser()

    const webSocketStore = useWebSocketStore()
    webSocketStore.closeConnection()

    isAuthenticated.value = true
    await fetchUserData()

    router.push('/home')
}

export function logout(router) {
    localStorage.removeItem('token')

    // Clear all stores
    const userStore = useUserStore()
    userStore.clearUser()
    
    const webSocketStore = useWebSocketStore()
    webSocketStore.closeConnection()

    isAuthenticated.value = false

    router.push('/')
}
