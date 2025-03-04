import { ref } from 'vue'
import axios from 'axios'
import { useUserStore } from '@/store/user'
import { useRoomsStore } from '@/store/rooms'
import { useApiStore} from '@/store/api'


async function fetchUserData() {
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

export const isAuthenticated = ref(!!localStorage.getItem('token'))

export async function login(token, router) {
    localStorage.setItem('token', token)
    localStorage.removeItem('rooms')

    isAuthenticated.value = true
    await fetchUserData()

    router.push('/home')
}

export function logout(router) {
    localStorage.removeItem('token')
    localStorage.removeItem('rooms')
    
    isAuthenticated.value = false

    router.push('/')
}
